/*
 * Copyright (C) 2008 Martin Willi
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
 * $Id: fetcher_manager.c 4591 2008-11-05 16:12:54Z martin $
 */

#include "fetcher_manager.h"

#include <debug.h>
#include <utils/mutex.h>
#include <utils/linked_list.h>

typedef struct private_fetcher_manager_t private_fetcher_manager_t;

/**
 * private data of fetcher_manager
 */
struct private_fetcher_manager_t {

	/**
	 * public functions
	 */
	fetcher_manager_t public;
	
	/**
	 * list of registered fetchers, as entry_t
	 */
	linked_list_t *fetchers;
	
	/**
	 * read write lock to list
	 */
	rwlock_t *lock;
};

typedef struct {
	/** assocaited fetcher construction function */
	fetcher_constructor_t create;
	/** URL this fetcher support */
	char *url;
} entry_t;

/**
 * destroy an entry_t
 */
static void entry_destroy(entry_t *entry)
{
	free(entry->url);
	free(entry);
}

/**
 * Implementation of fetcher_manager_t.fetch.
 */
static status_t fetch(private_fetcher_manager_t *this,
					  char *url, chunk_t *response, ...)
{
	enumerator_t *enumerator;
	status_t status = NOT_SUPPORTED;
	entry_t *entry;
	bool capable = FALSE;
	
	this->lock->read_lock(this->lock);
	enumerator = this->fetchers->create_enumerator(this->fetchers);
	while (enumerator->enumerate(enumerator, &entry))
	{
		fetcher_option_t opt;
		fetcher_t *fetcher;
		bool good = TRUE;
		va_list args;

		/* check URL support of fetcher */
		if (strncasecmp(entry->url, url, strlen(entry->url)))
		{
			continue;
		}
		/* create fetcher instance and set options */
		fetcher = entry->create();
		if (!fetcher)
		{
			continue;
		}
		va_start(args, response);
		while (good)
		{
			opt = va_arg(args, fetcher_option_t);
			switch (opt)
			{
				case FETCH_REQUEST_DATA:
					good = fetcher->set_option(fetcher, opt, va_arg(args, chunk_t));
					continue;
				case FETCH_REQUEST_TYPE:
					good = fetcher->set_option(fetcher, opt, va_arg(args, char*));
					continue;
				case FETCH_TIMEOUT:
					good = fetcher->set_option(fetcher, opt, va_arg(args, u_int));
					continue;
				case FETCH_END:
					break;;
			}
			break;
		}
		va_end(args);
		if (!good)
		{	/* fetcher does not support supplied options, try another */
			fetcher->destroy(fetcher);
			continue;
		}
		
		status = fetcher->fetch(fetcher, url, response);
		fetcher->destroy(fetcher);
		/* try another fetcher only if this one does not support that URL */
		if (status == NOT_SUPPORTED)
		{
			continue;
		}
		capable = TRUE;
		break;
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	if (!capable)
	{
		DBG1("unable to fetch from %s, no capable fetcher found", url);
	}
	return status;
}

/**
 * Implementation of fetcher_manager_t.add_fetcher.
 */
static void add_fetcher(private_fetcher_manager_t *this,	
						fetcher_constructor_t create, char *url)
{
	entry_t *entry = malloc_thing(entry_t);
	
	entry->url = strdup(url);
	entry->create = create;

	this->lock->write_lock(this->lock);
	this->fetchers->insert_last(this->fetchers, entry);
	this->lock->unlock(this->lock);
}

/**
 * Implementation of fetcher_manager_t.remove_fetcher.
 */
static void remove_fetcher(private_fetcher_manager_t *this,
						   fetcher_constructor_t create)
{
	enumerator_t *enumerator;
	entry_t *entry;
	
	this->lock->write_lock(this->lock);
	enumerator = this->fetchers->create_enumerator(this->fetchers);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->create == create)
		{
			this->fetchers->remove_at(this->fetchers, enumerator);
			entry_destroy(entry);
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
}

/**
 * Implementation of fetcher_manager_t.destroy
 */
static void destroy(private_fetcher_manager_t *this)
{
	this->fetchers->destroy_function(this->fetchers, (void*)entry_destroy);
	this->lock->destroy(this->lock);
	free(this);
}

/*
 * see header file
 */
fetcher_manager_t *fetcher_manager_create()
{
	private_fetcher_manager_t *this = malloc_thing(private_fetcher_manager_t);
	
	this->public.fetch = (status_t(*)(fetcher_manager_t*, char *url, chunk_t *response, ...))fetch;
	this->public.add_fetcher = (void(*)(fetcher_manager_t*, fetcher_constructor_t,char*))add_fetcher;
	this->public.remove_fetcher = (void(*)(fetcher_manager_t*, fetcher_constructor_t))remove_fetcher;
	this->public.destroy = (void(*)(fetcher_manager_t*))destroy;
	
	this->fetchers = linked_list_create();
	this->lock = rwlock_create(RWLOCK_DEFAULT);
	
	return &this->public;
}

