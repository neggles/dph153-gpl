/*
 * Copyright (C) 2007 Martin Willi
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
 * $Id: roam_job.c 3804 2008-04-14 11:37:46Z martin $
 */

#include <stdlib.h>

#include "roam_job.h"

#include <sa/ike_sa.h>
#include <daemon.h>


typedef struct private_roam_job_t private_roam_job_t;

/**
 * Private data of an roam_job_t Object
 */
struct private_roam_job_t {
	/**
	 * public roam_job_t interface
	 */
	roam_job_t public;
	
	/**
	 * has the address list changed, or the routing only?
	 */
	bool address;
};

/**
 * Implements job_t.destroy.
 */
static void destroy(private_roam_job_t *this)
{
	free(this);
}

/**
 * Implementation of job_t.execute. 
 */ 
static void execute(private_roam_job_t *this)
{
	ike_sa_t *ike_sa;
	linked_list_t *list;
	ike_sa_id_t *id;
	enumerator_t *enumerator;
	
	/* enumerator over all IKE_SAs gives us no way to checkin_and_destroy 
	 * after a DESTROY_ME, so we check out each available IKE_SA by hand. */
	list = linked_list_create();
	enumerator = charon->ike_sa_manager->create_enumerator(charon->ike_sa_manager);
	while (enumerator->enumerate(enumerator, &ike_sa))
	{
		id = ike_sa->get_id(ike_sa);
		list->insert_last(list, id->clone(id));
	}
	enumerator->destroy(enumerator);
	
	while (list->remove_last(list, (void**)&id) == SUCCESS)
	{
		ike_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager, id);
		if (ike_sa)
		{
			if (ike_sa->roam(ike_sa, this->address) == DESTROY_ME)
			{
				charon->ike_sa_manager->checkin_and_destroy(
											charon->ike_sa_manager, ike_sa);
			}
			else
			{
				charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
			}
		}
		id->destroy(id);
	}
	list->destroy(list);

	destroy(this);
}

/*
 * Described in header
 */
roam_job_t *roam_job_create(bool address)
{
	private_roam_job_t *this = malloc_thing(private_roam_job_t);
	
	this->public.job_interface.execute = (void (*) (job_t *)) execute;
	this->public.job_interface.destroy = (void (*) (job_t *)) destroy;
	
	this->address = address;

	return &this->public;
}

