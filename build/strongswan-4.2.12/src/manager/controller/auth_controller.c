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
 * $Id: auth_controller.c 3589 2008-03-13 14:14:44Z martin $
 */

#include "auth_controller.h"
#include "../manager.h"

#include <library.h>


typedef struct private_auth_controller_t private_auth_controller_t;

/**
 * private data of the task manager
 */
struct private_auth_controller_t {

	/**
	 * public functions
	 */
	auth_controller_t public;
	
	/**
	 * manager instance
	 */
	manager_t *manager;
};

static void login(private_auth_controller_t *this, request_t *request)
{
	request->set(request, "action", "check");
	request->set(request, "title", "Login");
	request->render(request, "templates/auth/login.cs");
}

static void check(private_auth_controller_t *this, request_t *request)
{
	char *username, *password;
	
	username = request->get_query_data(request, "username");
	password = request->get_query_data(request, "password");
	if (username && password &&
		this->manager->login(this->manager, username, password))
	{
		request->redirect(request, "ikesa/list");
	}
	else
	{
		request->redirect(request, "auth/login");
	}
}

static void logout(private_auth_controller_t *this, request_t *request)
{
	this->manager->logout(this->manager);
	request->redirect(request, "auth/login");
}

/**
 * Implementation of controller_t.get_name
 */
static char* get_name(private_auth_controller_t *this)
{
	return "auth";
}

/**
 * Implementation of controller_t.handle
 */
static void handle(private_auth_controller_t *this,
				   request_t *request, char *action)
{
	if (action)
	{
		if (streq(action, "login"))
		{
			return login(this, request);
		}
		else if (streq(action, "check")) 
		{
			return check(this, request);
		}
		else if (streq(action, "logout")) 
		{
			return logout(this, request);
		}
	}
	request->redirect(request, "auth/login");
}

/**
 * Implementation of controller_t.destroy
 */
static void destroy(private_auth_controller_t *this)
{
	free(this);
}

/*
 * see header file
 */
controller_t *auth_controller_create(context_t *context, void *param)
{
	private_auth_controller_t *this = malloc_thing(private_auth_controller_t);

	this->public.controller.get_name = (char*(*)(controller_t*))get_name;
	this->public.controller.handle = (void(*)(controller_t*,request_t*,char*,char*,char*,char*,char*))handle;
	this->public.controller.destroy = (void(*)(controller_t*))destroy;
	
	this->manager = (manager_t*)context;
	
	return &this->public.controller;
}

