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
 * $Id: medsrv_plugin.c 4137 2008-07-01 13:57:47Z martin $
 */

#include "medsrv_plugin.h"

#include "medsrv_creds.h"
#include "medsrv_config.h"

#include <daemon.h>

typedef struct private_medsrv_plugin_t private_medsrv_plugin_t;

/**
 * private data of medsrv plugin
 */
struct private_medsrv_plugin_t {

	/**
	 * implements plugin interface
	 */
	medsrv_plugin_t public;
	
	/**
	 * database connection instance
	 */
	database_t *db;
	
	/**
	 * medsrv credential set instance
	 */
	medsrv_creds_t *creds;
	
	/**
	 * medsrv config database
	 */
	medsrv_config_t *config;
};

/**
 * Implementation of plugin_t.destroy
 */
static void destroy(private_medsrv_plugin_t *this)
{
	charon->backends->remove_backend(charon->backends, &this->config->backend);
	charon->credentials->remove_set(charon->credentials, &this->creds->set);
	this->config->destroy(this->config);
	this->creds->destroy(this->creds);
	this->db->destroy(this->db);
	free(this);
}

/*
 * see header file
 */
plugin_t *plugin_create()
{
	char *uri;
	private_medsrv_plugin_t *this = malloc_thing(private_medsrv_plugin_t);
	
	this->public.plugin.destroy = (void(*)(plugin_t*))destroy;
	
	uri = lib->settings->get_str(lib->settings,
								 "medsrv.database", NULL);
	if (!uri)
	{
		DBG1(DBG_CFG, "mediation database URI not defined, skipped");
		free(this);
		return NULL;
	}
	
	this->db = lib->db->create(lib->db, uri);
	if (this->db == NULL)
	{
		DBG1(DBG_CFG, "opening mediation server database failed");
		free(this);
		return NULL;
	}
	
	this->creds = medsrv_creds_create(this->db);
	this->config = medsrv_config_create(this->db);
	
	charon->credentials->add_set(charon->credentials, &this->creds->set);
	charon->backends->add_backend(charon->backends, &this->config->backend);
	
	return &this->public.plugin;
}

