/*
 * Copyright (C) 2007-2008 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * $Id: connect_manager.c 4579 2008-11-05 11:29:56Z martin $
 */

#include "connect_manager.h"

#include <math.h>

#include <daemon.h>
#include <utils/mutex.h>
#include <utils/linked_list.h>
#include <crypto/hashers/hasher.h>

#include <processing/jobs/callback_job.h>
#include <processing/jobs/initiate_mediation_job.h>
#include <encoding/payloads/endpoint_notify.h>

/* base timeout
 * the check interval is ME_INTERVAL */
#define ME_INTERVAL 25 /* ms */
/* retransmission timeout is first ME_INTERVAL for ME_BOOST retransmissions
 * then gets reduced to ME_INTERVAL * ME_RETRANS_BASE ^ (sent retransmissions - ME_BOOST). */
/* number of initial retransmissions sent in short interval */
#define ME_BOOST 2
/* base for retransmissions */
#define ME_RETRANS_BASE 1.8
/* max number of retransmissions */
#define ME_MAX_RETRANS 13

/* time to wait before the initiator finishes the connectivity checks after
 * the first check has succeeded */
#define ME_WAIT_TO_FINISH 1000 /* ms */


typedef struct private_connect_manager_t private_connect_manager_t;

/**
 * Additional private members of connect_manager_t.
 */
struct private_connect_manager_t {
	/**
	 * Public interface of connect_manager_t.
	 */
	 connect_manager_t public;
	
	 /**
	  * Lock for exclusivly accessing the manager.
	  */
	 mutex_t *mutex;
	 
	 /**
	  * Hasher to generate signatures
	  */
	 hasher_t *hasher;
	 
	 /**
	  * Linked list with initiated mediated connections
	  */
	 linked_list_t *initiated;
	 
	 /**
	  * Linked list with checklists (hash table with connect ID as key would be better).
	  */
	 linked_list_t *checklists;
};

typedef enum check_state_t check_state_t;

enum check_state_t {
	CHECK_NONE,
	CHECK_WAITING,
	CHECK_IN_PROGRESS,
	CHECK_SUCCEEDED,
	CHECK_FAILED
};

typedef struct endpoint_pair_t endpoint_pair_t;

/**
 * An entry in the check list.
 */
struct endpoint_pair_t {
	/** pair id */
	u_int32_t id;
	
	/** priority */
	u_int64_t priority;
	
	/** local endpoint */
    host_t *local;
    
    /** remote endpoint */
    host_t *remote;
    
    /** state */
    check_state_t state;
    
    /** number of retransmissions */
    u_int32_t retransmitted;
    
    /** the generated packet */
    packet_t *packet;
};

/**
 * Destroys an endpoint pair
 */
static void endpoint_pair_destroy(endpoint_pair_t *this)
{
	DESTROY_IF(this->local);
    DESTROY_IF(this->remote);
    DESTROY_IF(this->packet);
	free(this);
}

/**
 * Creates a new entry for the list.
 */
static endpoint_pair_t *endpoint_pair_create(endpoint_notify_t *initiator,
		endpoint_notify_t *responder, bool initiator_is_local)
{
	endpoint_pair_t *this = malloc_thing(endpoint_pair_t);
	
	this->id = 0;
	
	u_int32_t pi = initiator->get_priority(initiator);
	u_int32_t pr = responder->get_priority(responder);
	this->priority = pow(2, 32) * min(pi, pr) + 2 * max(pi, pr) + (pi > pr ? 1 : 0);
	
	this->local = initiator_is_local ? initiator->get_base(initiator) : responder->get_base(responder);
	this->local = this->local->clone(this->local);
	this->remote = initiator_is_local ? responder->get_host(responder) : initiator->get_host(initiator);
	this->remote = this->remote->clone(this->remote);
	
	this->state = CHECK_WAITING;
	this->retransmitted = 0;
	this->packet = NULL;
    
	return this;
}


typedef struct check_list_t check_list_t;

/**
 * An entry in the linked list.
 */
struct check_list_t {
	
	struct {
		/** initiator's id */
		identification_t *id;
		
		/** initiator's key */
		chunk_t key;
		
		/** initiator's endpoints */
		linked_list_t *endpoints;
	} initiator;
	
	struct {
		/** responder's id */
		identification_t *id;
		
		/** responder's key */
		chunk_t key;
		
		/** responder's endpoints */
		linked_list_t *endpoints;
	} responder;
	
	/** connect id */
	chunk_t connect_id;
	
    /** list of endpoint pairs */
    linked_list_t *pairs;
    
    /** pairs queued for triggered checks */
    linked_list_t *triggered;
    
    /** state */
    check_state_t state;
    
    /** TRUE if this is the initiator */
	bool is_initiator;
	
	/** TRUE if the initiator is finishing the checks */
	bool is_finishing;
	
	/** the current sender job */
	job_t *sender;
	
};

/**
 * Destroys a checklist
 */
static void check_list_destroy(check_list_t *this)
{
	DESTROY_IF(this->initiator.id);
	DESTROY_IF(this->responder.id);
	
	chunk_free(&this->connect_id);
	chunk_free(&this->initiator.key);
	chunk_free(&this->responder.key);
	
	DESTROY_OFFSET_IF(this->initiator.endpoints, offsetof(endpoint_notify_t, destroy));
	DESTROY_OFFSET_IF(this->responder.endpoints, offsetof(endpoint_notify_t, destroy));
	
	DESTROY_FUNCTION_IF(this->pairs, (void*)endpoint_pair_destroy);
	/* this list contains some of the same elements as contained in this->pairs */
	DESTROY_IF(this->triggered); 
	
	free(this);
}

/**
 * Creates a new checklist
 */
static check_list_t *check_list_create(identification_t *initiator, identification_t *responder,
		chunk_t connect_id, chunk_t initiator_key, linked_list_t *initiator_endpoints,
		bool is_initiator)
{
	check_list_t *this = malloc_thing(check_list_t);
	
	this->connect_id = chunk_clone(connect_id);
	
	this->initiator.id = initiator->clone(initiator);
	this->initiator.key = chunk_clone(initiator_key);
	this->initiator.endpoints = initiator_endpoints->clone_offset(initiator_endpoints, offsetof(endpoint_notify_t, clone));
	
	this->responder.id = responder->clone(responder);
	this->responder.key = chunk_empty;
    this->responder.endpoints = NULL;
    
    this->pairs = linked_list_create();
    this->triggered = linked_list_create();
    this->state = CHECK_NONE;
    this->is_initiator = is_initiator;
    this->is_finishing = FALSE;
    
	return this;
}

typedef struct initiated_t initiated_t;

/**
 * For an initiator, the data stored about initiated mediation connections
 */
struct initiated_t {
	/** my id */
	identification_t *id;
	
	/** peer id */
	identification_t *peer_id;
	
	/** list of mediated sas */
	linked_list_t *mediated;
};

/**
 * Destroys a queued initiation
 */
static void initiated_destroy(initiated_t *this)
{
	DESTROY_IF(this->id);
	DESTROY_IF(this->peer_id);
	this->mediated->destroy_offset(this->mediated, offsetof(ike_sa_id_t, destroy));
	free(this);
}

/**
 * Creates a queued initiation
 */
static initiated_t *initiated_create(identification_t *id, identification_t *peer_id)
{
	initiated_t *this = malloc_thing(initiated_t);
	
	this->id = id->clone(id);
	this->peer_id = peer_id->clone(peer_id);
	this->mediated = linked_list_create();
    
	return this;
}


typedef struct check_t check_t;

/**
 * Data exchanged in a connectivity check
 */
struct check_t {
	/** message id */
	u_int32_t mid;
	
	/** source of the connectivity check */
	host_t *src;
	
	/** destination of the connectivity check */
	host_t *dst;
	
	/** connect id */
	chunk_t connect_id;
	
	/** endpoint */
	endpoint_notify_t *endpoint;
	
	/** raw endpoint payload (to verify the signature) */
	chunk_t endpoint_raw;
	
    /** connect auth */
    chunk_t auth;
};

/**
 * Destroys a connectivity check
 */
static void check_destroy(check_t *this)
{
	chunk_free(&this->connect_id);
	chunk_free(&this->endpoint_raw);
	chunk_free(&this->auth);
	DESTROY_IF(this->src);
	DESTROY_IF(this->dst);
	DESTROY_IF(this->endpoint);
	free(this);
}

/**
 * Creates a new connectivity check
 */
static check_t *check_create()
{
	check_t *this = malloc_thing(check_t);
	
	this->connect_id = chunk_empty;
	this->auth = chunk_empty;
	this->endpoint_raw = chunk_empty;
	this->src = NULL;
	this->dst = NULL;
	this->endpoint = NULL;
	
	this->mid = 0;
	
	return this;
}

typedef struct callback_data_t callback_data_t;

/**
 * Data required by several callback jobs used in this file
 */
struct callback_data_t {
	/** connect manager */
	private_connect_manager_t *connect_manager;
	
	/** connect id */
	chunk_t connect_id;
	
	/** message (pair) id */
	u_int32_t mid;
};

/**
 * Destroys a callback data object
 */
static void callback_data_destroy(callback_data_t *this)
{
	chunk_free(&this->connect_id);
	free(this);
}

/**
 * Creates a new callback data object
 */
static callback_data_t *callback_data_create(private_connect_manager_t *connect_manager,
		chunk_t connect_id)
{
	callback_data_t *this = malloc_thing(callback_data_t);	
	this->connect_manager = connect_manager;
	this->connect_id = chunk_clone(connect_id);
	this->mid = 0;
	return this;
}

/**
 * Creates a new retransmission data object
 */
static callback_data_t *retransmit_data_create(private_connect_manager_t *connect_manager,
		chunk_t connect_id, u_int32_t mid)
{
	callback_data_t *this = callback_data_create(connect_manager, connect_id);
	this->mid = mid;
	return this;
}

typedef struct initiate_data_t initiate_data_t;

/**
 * Data required by the initiate mediated
 */
struct initiate_data_t {
	/** checklist */
	check_list_t *checklist;
	
	/** waiting mediated connections */
	initiated_t *initiated;
};

/**
 * Destroys a initiate data object
 */
static void initiate_data_destroy(initiate_data_t *this)
{
	check_list_destroy(this->checklist);
	initiated_destroy(this->initiated);
	free(this);
}

/**
 * Creates a new initiate data object
 */
static initiate_data_t *initiate_data_create(check_list_t *checklist, initiated_t *initiated)
{
	initiate_data_t *this = malloc_thing(initiate_data_t);
	
	this->checklist = checklist;
	this->initiated = initiated;

	return this;
}

/**
 * Find an initiated connection by the peers' ids
 */
static bool match_initiated_by_ids(initiated_t *current, identification_t *id,
		identification_t *peer_id)
{
	return id->equals(id, current->id) && peer_id->equals(peer_id, current->peer_id);
}

static status_t get_initiated_by_ids(private_connect_manager_t *this,
		identification_t *id, identification_t *peer_id, initiated_t **initiated)
{
	return this->initiated->find_first(this->initiated,
				(linked_list_match_t)match_initiated_by_ids,
				(void**)initiated, id, peer_id);
}

/**
 * Removes data about initiated connections
 */
static void remove_initiated(private_connect_manager_t *this, initiated_t *initiated)
{
	iterator_t *iterator;
	initiated_t *current;
	
	iterator = this->initiated->create_iterator(this->initiated, TRUE);
	while (iterator->iterate(iterator, (void**)&current))
	{
		if (current == initiated)
		{
			iterator->remove(iterator);
			break;
		}
	}
	iterator->destroy(iterator);
}

/**
 * Find the checklist with a specific connect ID
 */
static bool match_checklist_by_id(check_list_t *current, chunk_t *connect_id)
{
	return chunk_equals(*connect_id, current->connect_id);
}

static status_t get_checklist_by_id(private_connect_manager_t *this,
		chunk_t connect_id, check_list_t **check_list)
{
	return this->checklists->find_first(this->checklists,
				(linked_list_match_t)match_checklist_by_id,
				(void**)check_list, &connect_id);
}

/**
 * Removes a checklist
 */
static void remove_checklist(private_connect_manager_t *this, check_list_t *checklist)
{
	iterator_t *iterator;
	check_list_t *current;
	
	iterator = this->checklists->create_iterator(this->checklists, TRUE);
	while (iterator->iterate(iterator, (void**)&current))
	{
		if (current == checklist)
		{
			iterator->remove(iterator);
			break;
		}
	}
	iterator->destroy(iterator);
}

/**
 * Checks if a list of endpoint_notify_t contains a certain host_t
 */
static bool match_endpoint_by_host(endpoint_notify_t *current, host_t *host)
{
	return host->equals(host, current->get_host(current));
}

static status_t endpoints_contain(linked_list_t *endpoints, host_t *host, endpoint_notify_t **endpoint)
{
	return endpoints->find_first(endpoints,
				(linked_list_match_t)match_endpoint_by_host,
				(void**)endpoint, host);
}

/**
 * Inserts an endpoint pair into the list of pairs ordered by priority (high to low)
 */
static void insert_pair_by_priority(linked_list_t *pairs, endpoint_pair_t *pair)
{
	iterator_t *iterator;
	endpoint_pair_t *current;
	bool inserted = FALSE;
	
	iterator = pairs->create_iterator(pairs, TRUE);
	while (iterator->iterate(iterator, (void**)&current))
	{
		if (current->priority < pair->priority)
		{
			iterator->insert_before(iterator, pair);
			inserted = TRUE;
			break;
		}
	}
	iterator->destroy(iterator);
	
	if (!inserted)
	{
		pairs->insert_last(pairs, pair);
	}
}

/**
 * Searches a list of endpoint_pair_t for a pair with specific host_ts
 */
static bool match_pair_by_hosts(endpoint_pair_t *current, host_t *local, host_t *remote)
{
	return local->equals(local, current->local) && remote->equals(remote, current->remote);
}

static status_t get_pair_by_hosts(linked_list_t *pairs, host_t *local, host_t *remote, endpoint_pair_t **pair)
{
	return pairs->find_first(pairs,
				(linked_list_match_t)match_pair_by_hosts,
				(void**)pair, local, remote);
}

static bool match_pair_by_id(endpoint_pair_t *current, u_int32_t *id)
{
	return current->id == *id;
}

/**
 * Searches for a pair with a specific id
 */
static status_t get_pair_by_id(check_list_t *checklist, u_int32_t id, endpoint_pair_t **pair)
{
	return checklist->pairs->find_first(checklist->pairs,
				(linked_list_match_t)match_pair_by_id,
				(void**)pair, &id);
}

static bool match_succeeded_pair(endpoint_pair_t *current)
{
	return current->state == CHECK_SUCCEEDED;
}

/**
 * Returns the best pair of state CHECK_SUCCEEDED from a checklist. 
 */
static status_t get_best_valid_pair(check_list_t *checklist, endpoint_pair_t **pair)
{
	return checklist->pairs->find_first(checklist->pairs,
				(linked_list_match_t)match_succeeded_pair,
				(void**)pair);
}

static bool match_waiting_pair(endpoint_pair_t *current)
{
	return current->state == CHECK_WAITING;
}

/**
 * Returns and *removes* the first triggered pair in state CHECK_WAITING. 
 */
static status_t get_triggered_pair(check_list_t *checklist, endpoint_pair_t **pair)
{
	iterator_t *iterator;
	endpoint_pair_t *current;
	status_t status = NOT_FOUND;
	
	iterator = checklist->triggered->create_iterator(checklist->triggered, TRUE);
	while (iterator->iterate(iterator, (void**)&current))
	{
		iterator->remove(iterator);
		
		if (current->state == CHECK_WAITING)
		{
			if (pair)
			{
				*pair = current;
			}
			status = SUCCESS;
			break;
		}
	}
	iterator->destroy(iterator);
	
	return status;
}

/**
 * Prints all the pairs on a checklist
 */
static void print_checklist(check_list_t *checklist)
{
	iterator_t *iterator;
	endpoint_pair_t *current;
	
	DBG1(DBG_IKE, "pairs on checklist %#B:", &checklist->connect_id);
	iterator = checklist->pairs->create_iterator(checklist->pairs, TRUE);
	while (iterator->iterate(iterator, (void**)&current))
	{
		DBG1(DBG_IKE, " * %#H - %#H (%d)", current->local, current->remote,
				current->priority);
	}
	iterator->destroy(iterator);
}

/**
 * Prunes identical pairs with lower priority from the list
 * Note: this function also numbers the remaining pairs serially
 */
static void prune_pairs(linked_list_t *pairs)
{
	iterator_t *iterator, *search;
	endpoint_pair_t *current, *other;
	u_int32_t id = 0;
	
	iterator = pairs->create_iterator(pairs, TRUE);
	search = pairs->create_iterator(pairs, TRUE);
	while (iterator->iterate(iterator, (void**)&current))
	{
		current->id = ++id;
		
		while (search->iterate(search, (void**)&other))
		{
			if (current == other)
			{
				continue;
			}
			
			if (current->local->equals(current->local, other->local) &&
					current->remote->equals(current->remote, other->remote))
			{
				/* since the list of pairs is sorted by priority in descending
				 * order, and we iterate the list from the beginning, we are
				 * sure that the priority of 'other' is lower than that of
				 * 'current', remove it */
				DBG1(DBG_IKE, "pruning endpoint pair %#H - %#H with priority %d",
						other->local, other->remote, other->priority);
				search->remove(search);
				endpoint_pair_destroy(other);
			}
		}
		search->reset(search);
	}
	search->destroy(search);
	iterator->destroy(iterator);
}

/**
 * Builds a list of endpoint pairs
 */
static void build_pairs(check_list_t *checklist)
{
	/* FIXME: limit endpoints and pairs */
	iterator_t *iterator_i, *iterator_r;
	endpoint_notify_t *initiator, *responder;
	
	iterator_i = checklist->initiator.endpoints->create_iterator(checklist->initiator.endpoints, TRUE);
	while (iterator_i->iterate(iterator_i, (void**)&initiator))
	{
		iterator_r = checklist->responder.endpoints->create_iterator(checklist->responder.endpoints, TRUE);
		while (iterator_r->iterate(iterator_r, (void**)&responder))
		{
			if (initiator->get_family(initiator) != responder->get_family(responder))
			{
				continue;
			}
			
			insert_pair_by_priority(checklist->pairs,
					endpoint_pair_create(initiator, responder, checklist->is_initiator));
		}
		iterator_r->destroy(iterator_r);
	}
	iterator_i->destroy(iterator_i);
	
	print_checklist(checklist);

	prune_pairs(checklist->pairs);
}

/**
 * Processes the payloads of a connectivity check and returns the extracted data
 */
static status_t process_payloads(message_t *message, check_t *check)
{
	iterator_t *iterator;
	payload_t *payload;

	iterator = message->get_payload_iterator(message);
	while (iterator->iterate(iterator, (void**)&payload))
	{
		if (payload->get_type(payload) != NOTIFY)
		{
			DBG1(DBG_IKE, "ignoring payload of type '%N' while processing "
					"connectivity check", payload_type_names, payload->get_type(payload));
			continue;
		}
		
		notify_payload_t *notify = (notify_payload_t*)payload;
		
		switch (notify->get_notify_type(notify))
		{
			case ME_ENDPOINT:
			{
				if (check->endpoint)
				{
					DBG1(DBG_IKE, "connectivity check contains multiple ME_ENDPOINT notifies");
					break;
				}
				
				endpoint_notify_t *endpoint = endpoint_notify_create_from_payload(notify);
				if (!endpoint)
				{
					DBG1(DBG_IKE, "received invalid ME_ENDPOINT notify");
					break;
				}
				check->endpoint = endpoint;
				check->endpoint_raw = chunk_clone(notify->get_notification_data(notify));
				DBG2(DBG_IKE, "received ME_ENDPOINT notify");
				break;
			}
			case ME_CONNECTID:
			{
				if (check->connect_id.ptr)
				{
					DBG1(DBG_IKE, "connectivity check contains multiple ME_CONNECTID notifies");
					break;
				}
				check->connect_id = chunk_clone(notify->get_notification_data(notify));
				DBG2(DBG_IKE, "received ME_CONNECTID %#B", &check->connect_id);
				break;
			}
			case ME_CONNECTAUTH:
			{
				if (check->auth.ptr)
				{
					DBG1(DBG_IKE, "connectivity check contains multiple ME_CONNECTAUTH notifies");
					break;
				}
				check->auth = chunk_clone(notify->get_notification_data(notify));
				DBG2(DBG_IKE, "received ME_CONNECTAUTH %#B", &check->auth);
				break;
			}
			default:
				break;
		}
	}
	iterator->destroy(iterator);
	
	if (!check->connect_id.ptr || !check->endpoint || !check->auth.ptr)
	{
		DBG1(DBG_IKE, "at least one payload was missing from the connectivity check");
		return FAILED;
	}
	
	return SUCCESS;
}

/**
 * Builds the signature for a connectivity check
 */
static chunk_t build_signature(private_connect_manager_t *this, 
		check_list_t *checklist, check_t *check, bool outbound)
{
	u_int32_t mid;
	chunk_t mid_chunk, key_chunk, sig_chunk;
	chunk_t sig_hash;
	
	mid = htonl(check->mid);
	mid_chunk = chunk_from_thing(mid);
	
	key_chunk = (checklist->is_initiator && outbound) || (!checklist->is_initiator && !outbound)
					? checklist->initiator.key : checklist->responder.key;
	
	/* signature = SHA1( MID | ME_CONNECTID | ME_ENDPOINT | ME_CONNECTKEY ) */
	sig_chunk = chunk_cat("cccc", mid_chunk, check->connect_id, check->endpoint_raw, key_chunk);
	this->hasher->allocate_hash(this->hasher, sig_chunk, &sig_hash);
	DBG3(DBG_IKE, "sig_chunk %#B", &sig_chunk);
	DBG3(DBG_IKE, "sig_hash %#B", &sig_hash);
	
	chunk_free(&sig_chunk);
	return sig_hash;
}

static void queue_retransmission(private_connect_manager_t *this, check_list_t *checklist, endpoint_pair_t *pair);
static void schedule_checks(private_connect_manager_t *this, check_list_t *checklist, u_int32_t time);
static void finish_checks(private_connect_manager_t *this, check_list_t *checklist);

/**
 * After one of the initiator's pairs has succeeded we finish the checks without
 * waiting for all the timeouts  
 */
static job_requeue_t initiator_finish(callback_data_t *data)
{
	private_connect_manager_t *this = data->connect_manager;

	this->mutex->lock(this->mutex);

	check_list_t *checklist;
	if (get_checklist_by_id(this, data->connect_id, &checklist) != SUCCESS)
	{
		DBG1(DBG_IKE, "checklist with id '%#B' not found, can't finish connectivity checks",
				&data->connect_id);
		this->mutex->unlock(this->mutex);
		return JOB_REQUEUE_NONE;
	}
	
	finish_checks(this, checklist);
	
	this->mutex->unlock(this->mutex);
	
	return JOB_REQUEUE_NONE;
}

/**
 * Updates the state of the whole checklist
 */
static void update_checklist_state(private_connect_manager_t *this, check_list_t *checklist)
{
	iterator_t *iterator;
	endpoint_pair_t *current;
	bool in_progress = FALSE, succeeded = FALSE;

	iterator = checklist->pairs->create_iterator(checklist->pairs, TRUE);
	while (iterator->iterate(iterator, (void**)&current))
	{
		switch(current->state)
		{
			case CHECK_WAITING:
				/* at least one is still waiting -> checklist remains
				 * in waiting state */
				iterator->destroy(iterator);
				return;
			case CHECK_IN_PROGRESS:
				in_progress = TRUE;
				break;
			case CHECK_SUCCEEDED:
				succeeded = TRUE;
				break;
			default:
				break;
		}
	}
	iterator->destroy(iterator);
	
	if (checklist->is_initiator && succeeded && !checklist->is_finishing)
	{
		/* instead of waiting until all checks have finished (i.e. all
		 * retransmissions have failed) the initiator finishes the checks
		 * right after the first check has succeeded. to allow a probably
		 * better pair to succeed, we still wait a certain time */
		DBG2(DBG_IKE, "fast finishing checks for checklist '%#B'", &checklist->connect_id);
		
		callback_data_t *data = callback_data_create(this, checklist->connect_id);
		job_t *job = (job_t*)callback_job_create((callback_job_cb_t)initiator_finish, data, (callback_job_cleanup_t)callback_data_destroy, NULL);
		charon->scheduler->schedule_job(charon->scheduler, job, ME_WAIT_TO_FINISH);
		checklist->is_finishing = TRUE;
	}
	
	if (in_progress)
	{
		checklist->state = CHECK_IN_PROGRESS;
	}
	else if (succeeded)
	{
		checklist->state = CHECK_SUCCEEDED;
	}
	else
	{
		checklist->state = CHECK_FAILED;
	}
}

/**
 * This function is triggered for each sent check after a specific timeout
 */
static job_requeue_t retransmit(callback_data_t *data)
{
	private_connect_manager_t *this = data->connect_manager;
	
	this->mutex->lock(this->mutex);

	check_list_t *checklist;
	if (get_checklist_by_id(this, data->connect_id, &checklist) != SUCCESS)
	{
		DBG1(DBG_IKE, "checklist with id '%#B' not found, can't retransmit connectivity check",
				&data->connect_id);
		this->mutex->unlock(this->mutex);
		return JOB_REQUEUE_NONE;
	}
	
	endpoint_pair_t *pair;
	if (get_pair_by_id(checklist, data->mid, &pair) != SUCCESS)
	{
		DBG1(DBG_IKE, "pair with id '%d' not found, can't retransmit connectivity check",
				data->mid);
		goto retransmit_end;
	}
	
	if (pair->state != CHECK_IN_PROGRESS)
	{
		DBG2(DBG_IKE, "pair with id '%d' is in wrong state [%d], don't retransmit the connectivity check",
				data->mid, pair->state);
		goto retransmit_end;
	}
	
	if (++pair->retransmitted > ME_MAX_RETRANS)
	{
		DBG2(DBG_IKE, "pair with id '%d' failed after %d retransmissions",
				data->mid, ME_MAX_RETRANS);
		pair->state = CHECK_FAILED;
		goto retransmit_end;
	}
	
	charon->sender->send(charon->sender, pair->packet->clone(pair->packet));
	
	queue_retransmission(this, checklist, pair);

retransmit_end:
	update_checklist_state(this, checklist);
	
	switch(checklist->state)
	{
		case CHECK_SUCCEEDED:
		case CHECK_FAILED:
			finish_checks(this, checklist);
			break;
		default:
			break;
	}
	
	this->mutex->unlock(this->mutex);
	
	/* we reschedule it manually */
	return JOB_REQUEUE_NONE;
}

/**
 * Queues a retransmission job
 */
static void queue_retransmission(private_connect_manager_t *this, check_list_t *checklist, endpoint_pair_t *pair)
{
	callback_data_t *data = retransmit_data_create(this, checklist->connect_id, pair->id);
	job_t *job = (job_t*)callback_job_create((callback_job_cb_t)retransmit, data, (callback_job_cleanup_t)callback_data_destroy, NULL);
	
	u_int32_t retransmission = pair->retransmitted + 1;
	u_int32_t rto = ME_INTERVAL;
	if (retransmission > ME_BOOST)
	{
		rto = (u_int32_t)(ME_INTERVAL * pow(ME_RETRANS_BASE, retransmission - ME_BOOST));
	}
	DBG2(DBG_IKE, "scheduling retransmission %d of pair '%d' in %dms", retransmission, pair->id, rto);
	
	charon->scheduler->schedule_job(charon->scheduler, (job_t*)job, rto);
}

/**
 * Sends a check
 */
static void send_check(private_connect_manager_t *this, check_list_t *checklist,
		check_t *check, endpoint_pair_t *pair, bool request)
{
	message_t *message = message_create();
	message->set_message_id(message, check->mid);
	message->set_exchange_type(message, INFORMATIONAL);
	message->set_request(message, request);
	message->set_destination(message, check->dst->clone(check->dst));
	message->set_source(message, check->src->clone(check->src));
	
	ike_sa_id_t *ike_sa_id = ike_sa_id_create(0, 0, request);
	message->set_ike_sa_id(message, ike_sa_id);
	ike_sa_id->destroy(ike_sa_id);

	message->add_notify(message, FALSE, ME_CONNECTID, check->connect_id);
	DBG2(DBG_IKE, "send ME_CONNECTID %#B", &check->connect_id);
	
	notify_payload_t *endpoint = check->endpoint->build_notify(check->endpoint);
	check->endpoint_raw = chunk_clone(endpoint->get_notification_data(endpoint));
	message->add_payload(message, (payload_t*)endpoint);
	DBG2(DBG_IKE, "send ME_ENDPOINT notify");
	
	check->auth = build_signature(this, checklist, check, TRUE);
	message->add_notify(message, FALSE, ME_CONNECTAUTH, check->auth);
	DBG2(DBG_IKE, "send ME_CONNECTAUTH %#B", &check->auth);
	
	packet_t *packet;
	if (message->generate(message, NULL, NULL, &packet) == SUCCESS)
	{
		charon->sender->send(charon->sender, packet->clone(packet));
		
		if (request)
		{
			DESTROY_IF(pair->packet);
			pair->packet = packet;
			pair->retransmitted = 0;
			queue_retransmission(this, checklist, pair);
		}
		else
		{
			packet->destroy(packet);
		}
	}
	message->destroy(message);
}

/**
 * Queues a triggered check
 */
static void queue_triggered_check(private_connect_manager_t *this, 
		check_list_t *checklist, endpoint_pair_t *pair)
{
	DBG2(DBG_IKE, "queueing triggered check for pair '%d'", pair->id);
 	pair->state = CHECK_WAITING;
 	checklist->triggered->insert_last(checklist->triggered, pair);
 	
 	if (!checklist->sender)
 	{
 		/* if the sender is not running we restart it */
 		schedule_checks(this, checklist, ME_INTERVAL);
 	}
}

/**
 * This function is triggered for each checklist at a specific interval
 */
static job_requeue_t sender(callback_data_t *data)
{
	private_connect_manager_t *this = data->connect_manager;

	this->mutex->lock(this->mutex);
	
	check_list_t *checklist;
	if (get_checklist_by_id(this, data->connect_id, &checklist) != SUCCESS)
	{
		DBG1(DBG_IKE, "checklist with id '%#B' not found, can't send connectivity check",
				&data->connect_id);
		this->mutex->unlock(this->mutex);
		return JOB_REQUEUE_NONE;
	}
	
	/* reset the sender */
	checklist->sender = NULL;
	
	endpoint_pair_t *pair;
	if (get_triggered_pair(checklist, &pair) != SUCCESS)
	{
		DBG1(DBG_IKE, "no triggered check queued, sending an ordinary check");
		
		if (checklist->pairs->find_first(checklist->pairs,
				(linked_list_match_t)match_waiting_pair, (void**)&pair) != SUCCESS)
		{
			this->mutex->unlock(this->mutex);
			DBG1(DBG_IKE, "no pairs in waiting state, aborting");
			return JOB_REQUEUE_NONE;
		}
	}
	else
	{
		DBG1(DBG_IKE, "triggered check found");
	}

	check_t *check = check_create();
	check->mid = pair->id;
	check->src = pair->local->clone(pair->local);
	check->dst = pair->remote->clone(pair->remote);
	check->connect_id = chunk_clone(checklist->connect_id);
	check->endpoint = endpoint_notify_create();
	
	pair->state = CHECK_IN_PROGRESS;
	
	send_check(this, checklist, check, pair, TRUE);
	
	check_destroy(check);
	
	/* schedule this job again */
	schedule_checks(this, checklist, ME_INTERVAL);
	
	this->mutex->unlock(this->mutex);
	
	/* we reschedule it manually */
	return JOB_REQUEUE_NONE;
}

/**
 * Schedules checks for a checklist (time in ms)
 */
static void schedule_checks(private_connect_manager_t *this, check_list_t *checklist, u_int32_t time)
{
	callback_data_t *data = callback_data_create(this, checklist->connect_id);
	checklist->sender = (job_t*)callback_job_create((callback_job_cb_t)sender, data, (callback_job_cleanup_t)callback_data_destroy, NULL);
	charon->scheduler->schedule_job(charon->scheduler, checklist->sender, time);
}

/**
 * Initiates waiting mediated connections
 */
static job_requeue_t initiate_mediated(initiate_data_t *data)
{
	check_list_t *checklist = data->checklist;
	initiated_t *initiated = data->initiated;
	
	endpoint_pair_t *pair;
	if (get_best_valid_pair(checklist, &pair) == SUCCESS)
	{
		ike_sa_id_t *waiting_sa;
		iterator_t *iterator = initiated->mediated->create_iterator(initiated->mediated, TRUE);
		while (iterator->iterate(iterator, (void**)&waiting_sa))
		{
			ike_sa_t *sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager, waiting_sa);
			if (sa->initiate_mediated(sa, pair->local, pair->remote, checklist->connect_id) != SUCCESS)
			{
				DBG1(DBG_IKE, "establishing mediated connection failed");
				charon->ike_sa_manager->checkin_and_destroy(charon->ike_sa_manager, sa);
			}
			charon->ike_sa_manager->checkin(charon->ike_sa_manager, sa);
		}
		iterator->destroy(iterator);
	}
	else
	{
		/* this should (can?) not happen */
	}
	
	return JOB_REQUEUE_NONE;
}

/**
 * Finishes checks for a checklist
 */
static void finish_checks(private_connect_manager_t *this, check_list_t *checklist)
{
	if (checklist->is_initiator)
	{
		initiated_t *initiated;
		if (get_initiated_by_ids(this, checklist->initiator.id,
				checklist->responder.id, &initiated) == SUCCESS)
		{
			remove_checklist(this, checklist);
			remove_initiated(this, initiated);
			
			initiate_data_t *data = initiate_data_create(checklist, initiated);
			job_t *job = (job_t*)callback_job_create((callback_job_cb_t)initiate_mediated, data, (callback_job_cleanup_t)initiate_data_destroy, NULL);
			charon->processor->queue_job(charon->processor, job);
			return;
		}
		else
		{
			DBG1(DBG_IKE, "there is no mediated connection waiting between '%D' "
					"and '%D'", checklist->initiator.id, checklist->responder.id);
		}
	}
}

/**
 * Process the response to one of our requests
 */
static void process_response(private_connect_manager_t *this, check_t *check,
		check_list_t *checklist)
{
	endpoint_pair_t *pair;
	if (get_pair_by_id(checklist, check->mid, &pair) == SUCCESS)
	{
		if (pair->local->equals(pair->local, check->dst) &&
				pair->remote->equals(pair->remote, check->src))
		{
			DBG1(DBG_IKE, "endpoint pair '%d' is valid: '%#H' - '%#H'", pair->id,
					pair->local, pair->remote);
			pair->state = CHECK_SUCCEEDED;
		}
		
		linked_list_t *local_endpoints = checklist->is_initiator ?
			checklist->initiator.endpoints : checklist->responder.endpoints;
		
		endpoint_notify_t *local_endpoint;
		if (endpoints_contain(local_endpoints,
				check->endpoint->get_host(check->endpoint), &local_endpoint) != SUCCESS)
		{
			local_endpoint = endpoint_notify_create_from_host(PEER_REFLEXIVE,
					check->endpoint->get_host(check->endpoint), pair->local);
			local_endpoint->set_priority(local_endpoint, check->endpoint->get_priority(check->endpoint));
			local_endpoints->insert_last(local_endpoints, local_endpoint);
		}
		
		update_checklist_state(this, checklist);
		
		switch(checklist->state)
		{
			case CHECK_SUCCEEDED:
			case CHECK_FAILED:
				finish_checks(this, checklist);
				break;
			default:
				break;
		}
	}
	else
	{
		DBG1(DBG_IKE, "pair with id '%d' not found", check->mid);
	}
}

static void process_request(private_connect_manager_t *this, check_t *check,
		check_list_t *checklist)
{
	linked_list_t *remote_endpoints = checklist->is_initiator ?
				checklist->responder.endpoints : checklist->initiator.endpoints;
		
	endpoint_notify_t *peer_reflexive, *remote_endpoint;
	peer_reflexive = endpoint_notify_create_from_host(PEER_REFLEXIVE, check->src, NULL);
	peer_reflexive->set_priority(peer_reflexive, check->endpoint->get_priority(check->endpoint));
		
	if (endpoints_contain(remote_endpoints, check->src, &remote_endpoint) != SUCCESS)
	{
		remote_endpoint = peer_reflexive->clone(peer_reflexive);
		remote_endpoints->insert_last(remote_endpoints, remote_endpoint);
	}
	
	endpoint_pair_t *pair;
	if (get_pair_by_hosts(checklist->pairs, check->dst, check->src, &pair) == SUCCESS)
	{
		switch(pair->state)
		{
			case CHECK_IN_PROGRESS:
				/* prevent retransmissions */
				pair->retransmitted = ME_MAX_RETRANS;
				/* FIXME: we should wait to the next rto to send the triggered check
				 * fall-through */
			case CHECK_WAITING:
			case CHECK_FAILED:
				queue_triggered_check(this, checklist, pair);
				break;
			case CHECK_SUCCEEDED:
			default:
				break;
		}
	}
	else
	{
		endpoint_notify_t *local_endpoint = endpoint_notify_create_from_host(HOST, check->dst, NULL);
		
		endpoint_notify_t *initiator = checklist->is_initiator ? local_endpoint : remote_endpoint;
		endpoint_notify_t *responder = checklist->is_initiator ? remote_endpoint : local_endpoint;
		
		pair = endpoint_pair_create(initiator, responder, checklist->is_initiator);
		pair->id = checklist->pairs->get_count(checklist->pairs) + 1;
		
		insert_pair_by_priority(checklist->pairs, pair);
		
		queue_triggered_check(this, checklist, pair);
		
		local_endpoint->destroy(local_endpoint);
	}
	
	
	check_t *response = check_create();
	
	response->mid = check->mid;
	response->src = check->dst->clone(check->dst);
	response->dst = check->src->clone(check->src);
	response->connect_id = chunk_clone(check->connect_id);
	response->endpoint = peer_reflexive;
	
	send_check(this, checklist, response, pair, FALSE);
	
	check_destroy(response);
}

/**
 * Implementation of connect_manager_t.process_check.
 */
static void process_check(private_connect_manager_t *this, message_t *message)
{
	if (message->parse_body(message, NULL, NULL) != SUCCESS)
	{
		DBG1(DBG_IKE, "%N %s with message ID %d processing failed",
			 exchange_type_names, message->get_exchange_type(message),
			 message->get_request(message) ? "request" : "response",
			 message->get_message_id(message));
		return;
	}
	
	check_t *check = check_create();
	check->mid = message->get_message_id(message);
	check->src = message->get_source(message);
	check->src = check->src->clone(check->src);
	check->dst = message->get_destination(message);
	check->dst = check->dst->clone(check->dst);
	
	if (process_payloads(message, check) != SUCCESS)
	{
		DBG1(DBG_IKE, "invalid connectivity check %s received",
				message->get_request(message) ? "request" : "response");
		check_destroy(check);
		return;
	}
	
	this->mutex->lock(this->mutex);
	
	check_list_t *checklist;
	if (get_checklist_by_id(this, check->connect_id, &checklist) != SUCCESS)
	{
		DBG1(DBG_IKE, "checklist with id '%#B' not found",
				&check->connect_id);
		check_destroy(check);
		this->mutex->unlock(this->mutex);
		return;
	}
	
	chunk_t sig = build_signature(this, checklist, check, FALSE); 
	if (!chunk_equals(sig, check->auth))
	{
		DBG1(DBG_IKE, "connectivity check verification failed");
		check_destroy(check);
		chunk_free(&sig);
		this->mutex->unlock(this->mutex);
		return;
	}
	chunk_free(&sig);
	
	if (message->get_request(message))
	{
		process_request(this, check, checklist);
	}
	else
	{
		process_response(this, check, checklist);
	}
	
	this->mutex->unlock(this->mutex);
	
	check_destroy(check);
}

/**
 * Implementation of connect_manager_t.check_and_register.
 */
static bool check_and_register(private_connect_manager_t *this,
			identification_t *id, identification_t *peer_id,
			ike_sa_id_t *mediated_sa)
{
	initiated_t *initiated;
	bool already_there = TRUE;

	this->mutex->lock(this->mutex);

	if (get_initiated_by_ids(this, id, peer_id, &initiated) != SUCCESS)
	{
		DBG2(DBG_IKE, "registered waiting mediated connection with '%D'", peer_id);
		initiated = initiated_create(id, peer_id);
		this->initiated->insert_last(this->initiated, initiated);
		already_there = FALSE;
	}
	
	if (initiated->mediated->find_first(initiated->mediated, 
			(linked_list_match_t)mediated_sa->equals, NULL, mediated_sa) != SUCCESS)
	{
		initiated->mediated->insert_last(initiated->mediated, mediated_sa->clone(mediated_sa));
	}

	this->mutex->unlock(this->mutex);

	return already_there;
}

/**
 * Implementation of connect_manager_t.check_and_initiate.
 */
static void check_and_initiate(private_connect_manager_t *this, ike_sa_id_t *mediation_sa,
		identification_t *id, identification_t *peer_id)
{
	initiated_t *initiated;

	this->mutex->lock(this->mutex);

	if (get_initiated_by_ids(this, id, peer_id, &initiated) != SUCCESS)
	{
		DBG2(DBG_IKE, "no waiting mediated connections with '%D'", peer_id);
		this->mutex->unlock(this->mutex);
		return;
	}
	
	ike_sa_id_t *waiting_sa;
	iterator_t *iterator = initiated->mediated->create_iterator(initiated->mediated, TRUE);
	while (iterator->iterate(iterator, (void**)&waiting_sa))
	{
		job_t *job = (job_t*)reinitiate_mediation_job_create(mediation_sa, waiting_sa);
		charon->processor->queue_job(charon->processor, job);
	}
	iterator->destroy(iterator);

	this->mutex->unlock(this->mutex);
}

/**
 * Implementation of connect_manager_t.set_initiator_data.
 */
static status_t set_initiator_data(private_connect_manager_t *this,
		identification_t *initiator, identification_t *responder,
		chunk_t connect_id, chunk_t key, linked_list_t *endpoints, bool is_initiator)
{
	check_list_t *checklist;
	
	this->mutex->lock(this->mutex);	
	
	if (get_checklist_by_id(this, connect_id, NULL) == SUCCESS)
	{
		DBG1(DBG_IKE, "checklist with id '%#B' already exists, aborting",
				&connect_id);
		this->mutex->unlock(this->mutex);
		return FAILED;
	}
	
	checklist = check_list_create(initiator, responder, connect_id, key, endpoints, is_initiator);
	this->checklists->insert_last(this->checklists, checklist);
	
	this->mutex->unlock(this->mutex);
	
	return SUCCESS;
}

/**
 * Implementation of connect_manager_t.set_responder_data.
 */
static status_t set_responder_data(private_connect_manager_t *this,
		chunk_t connect_id, chunk_t key, linked_list_t *endpoints)
{
	check_list_t *checklist;

	this->mutex->lock(this->mutex);
	
	if (get_checklist_by_id(this, connect_id, &checklist) != SUCCESS)
	{
		DBG1(DBG_IKE, "checklist with id '%#B' not found",
				&connect_id);
		this->mutex->unlock(this->mutex);
		return NOT_FOUND;
	}
	
	checklist->responder.key = chunk_clone(key);
	checklist->responder.endpoints = endpoints->clone_offset(endpoints, offsetof(endpoint_notify_t, clone));
	checklist->state = CHECK_WAITING;
	
	build_pairs(checklist);
	
	/* send the first check immediately */
	schedule_checks(this, checklist, 0);
	
	this->mutex->unlock(this->mutex);
	
	return SUCCESS;
}

/**
 * Implementation of connect_manager_t.stop_checks.
 */
static status_t stop_checks(private_connect_manager_t *this, chunk_t connect_id)
{
	check_list_t *checklist;

	this->mutex->lock(this->mutex);
	
	if (get_checklist_by_id(this, connect_id, &checklist) != SUCCESS)
	{
		DBG1(DBG_IKE, "checklist with id '%#B' not found",
				&connect_id);
		this->mutex->unlock(this->mutex);
		return NOT_FOUND;
	}
	
	DBG1(DBG_IKE, "removing checklist with id '%#B'", &connect_id);
	
	remove_checklist(this, checklist);
	check_list_destroy(checklist);
	
	this->mutex->unlock(this->mutex);
	
	return SUCCESS;
}

/**
 * Implementation of connect_manager_t.destroy.
 */
static void destroy(private_connect_manager_t *this)
{
	this->mutex->lock(this->mutex);
	
	this->hasher->destroy(this->hasher);
	this->checklists->destroy_function(this->checklists, (void*)check_list_destroy);
	this->initiated->destroy_function(this->initiated, (void*)initiated_destroy);
	
	this->mutex->unlock(this->mutex);	
	this->mutex->destroy(this->mutex);
	free(this);
}

/*
 * Described in header.
 */
connect_manager_t *connect_manager_create()
{
	private_connect_manager_t *this = malloc_thing(private_connect_manager_t);

	this->public.destroy = (void(*)(connect_manager_t*))destroy;
	this->public.check_and_register = (bool(*)(connect_manager_t*,identification_t*,identification_t*,ike_sa_id_t*))check_and_register;
	this->public.check_and_initiate = (void(*)(connect_manager_t*,ike_sa_id_t*,identification_t*,identification_t*))check_and_initiate;
	this->public.set_initiator_data = (status_t(*)(connect_manager_t*,identification_t*,identification_t*,chunk_t,chunk_t,linked_list_t*,bool))set_initiator_data;
	this->public.set_responder_data = (status_t(*)(connect_manager_t*,chunk_t,chunk_t,linked_list_t*))set_responder_data;
	this->public.process_check = (void(*)(connect_manager_t*,message_t*))process_check;
	this->public.stop_checks = (status_t(*)(connect_manager_t*,chunk_t))stop_checks;
	
	this->hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
	if (this->hasher == NULL)
	{
		DBG1(DBG_IKE, "unable to create connect manager, SHA1 not supported");
		free(this);
		return NULL;
	}
	
	this->checklists = linked_list_create();
	this->initiated = linked_list_create();
	
	this->mutex = mutex_create(MUTEX_DEFAULT);
	
	return (connect_manager_t*)this;
}
