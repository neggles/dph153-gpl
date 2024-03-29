/*
 * Copyright (C) 2008 Martin Willi
 * Copyright (C) 2007 Andreas Steffen
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
 * $Id: ldap_fetcher.c 3693 2008-03-28 22:44:45Z andreas $
 */

#ifndef LDAP_DEPRECATED
#define LDAP_DEPRECATED	1
#endif /* LDAP_DEPRECATED */
#include <ldap.h>

#include <errno.h>

#include <library.h>
#include <debug.h>

#include "ldap_fetcher.h"

#define DEFAULT_TIMEOUT 10

typedef struct private_ldap_fetcher_t private_ldap_fetcher_t;

/**
 * Private Data of a ldap_fetcher_t object.
 */
struct private_ldap_fetcher_t {
	/**
	 * Public data
	 */
	ldap_fetcher_t public;
	
	/**
	 * timeout to use for fetches
	 */
	u_int timeout;
};

/**
 * Parses the result returned by an ldap query
 */
static bool parse(LDAP *ldap, LDAPMessage *result, chunk_t *response)
{
	LDAPMessage *entry = ldap_first_entry(ldap, result);
	bool success = FALSE;

	if (entry)
	{
		BerElement *ber = NULL;
		char *attr;

		attr = ldap_first_attribute(ldap, entry, &ber);
		if (attr)
		{
			struct berval **values = ldap_get_values_len(ldap, entry, attr);

			if (values)
			{
				if (values[0])
				{
					*response = chunk_alloc(values[0]->bv_len);
					memcpy(response->ptr, values[0]->bv_val, response->len);
					success = TRUE;
				}
				else
				{
					DBG1("LDAP response contains no values");
				}
				ldap_value_free_len(values);
			}
			else
			{
				DBG1("getting LDAP values failed: %s", 
					 ldap_err2string(ldap_result2error(ldap, entry, 0)));
			}
			ldap_memfree(attr);
		}
		else
		{
			DBG1("finding LDAP attributes failed: %s",
				 ldap_err2string(ldap_result2error(ldap, entry, 0)));
		}
		ber_free(ber, 0);
	}
	else
	{
		DBG1("finding first LDAP entry failed: %s",
			 ldap_err2string(ldap_result2error(ldap, entry, 0)));
	}
	return success;
}


static status_t fetch(private_ldap_fetcher_t *this, char *url,
					  chunk_t *result, va_list args)
{
	LDAP *ldap;
	LDAPURLDesc *lurl;
	LDAPMessage *msg;
	int res;
	int ldap_version = LDAP_VERSION3;
	struct timeval timeout;
	status_t status = FAILED;
	
	if (!strneq(url, "ldap", 4))
	{
		return NOT_SUPPORTED;
	}
	if (ldap_url_parse(url, &lurl) != LDAP_SUCCESS)
	{
		return NOT_SUPPORTED;
	}
	ldap = ldap_init(lurl->lud_host, lurl->lud_port);
	if (ldap == NULL)
	{
		DBG1("LDAP initialization failed: %s", strerror(errno));
		ldap_free_urldesc(lurl);
		return FAILED;
	}
	
	timeout.tv_sec = this->timeout;
	timeout.tv_usec = 0;

	ldap_set_option(ldap, LDAP_OPT_PROTOCOL_VERSION, &ldap_version);
	ldap_set_option(ldap, LDAP_OPT_NETWORK_TIMEOUT, &timeout);

	DBG2("sending LDAP request to '%s'...", url);

	res = ldap_simple_bind_s(ldap, NULL, NULL);
	if (res == LDAP_SUCCESS)
	{
		res = ldap_search_st(ldap, lurl->lud_dn, lurl->lud_scope,
							 lurl->lud_filter, lurl->lud_attrs,
							 0, &timeout, &msg);

		if (res == LDAP_SUCCESS)
		{
			if (parse(ldap, msg, result))
			{
				status = SUCCESS;
			}
			ldap_msgfree(msg);
		}
		else
		{
			DBG1("LDAP search failed: %s", ldap_err2string(res));
		}
	}
	else
	{
		DBG1("LDAP bind to '%s' failed: %s", url, ldap_err2string(res));
	}
	ldap_unbind_s(ldap);
	ldap_free_urldesc(lurl);
	return status;
}


/**
 * Implementation of fetcher_t.set_option.
 */
static bool set_option(private_ldap_fetcher_t *this, fetcher_option_t option, ...)
{
	va_list args;
	
	va_start(args, option);
	switch (option)
	{
		case FETCH_TIMEOUT:
		{
			this->timeout = va_arg(args, u_int);
			return TRUE;
		}
		default:
			return FALSE;
	}
}

/**
 * Implements ldap_fetcher_t.destroy
 */
static void destroy(private_ldap_fetcher_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
ldap_fetcher_t *ldap_fetcher_create()
{
	private_ldap_fetcher_t *this = malloc_thing(private_ldap_fetcher_t);

	this->public.interface.fetch = (status_t(*)(fetcher_t*,char*,chunk_t*))fetch;
	this->public.interface.set_option = (bool(*)(fetcher_t*, fetcher_option_t option, ...))set_option;
	this->public.interface.destroy = (void (*)(fetcher_t*))destroy;
	
	this->timeout = DEFAULT_TIMEOUT;
	
	return &this->public;
}

