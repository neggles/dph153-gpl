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
 * $Id$
 */

/**
 * @defgroup nm_creds nm_creds
 * @{ @ingroup nm
 */

#ifndef NM_CREDS_H_
#define NM_CREDS_H_

#include <credentials/credential_set.h>

typedef struct nm_creds_t nm_creds_t;

/**
 * NetworkManager credentials helper.
 */
struct nm_creds_t {

	/**
	 * Implements credential_set_t
	 */
	credential_set_t set;
	
	/**
	 * Set the trusted gateway certificate to serve by this set.
	 *
	 * @param cert		certificate to serve
	 */
	void (*set_certificate)(nm_creds_t *this, certificate_t *cert);
	
	/**
	 * Set the username/password for authentication.
	 *
	 * @param id		ID of the user
	 * @param password	password to use for authentication
	 */
	void (*set_username_password)(nm_creds_t *this, identification_t *id,
								  char *password);
	/**
	 * Set the certificate and private key to use for client authentication.
	 *
	 * @param cert		client certificate
	 * @param key		associated private key
	 */
	void (*set_cert_and_key)(nm_creds_t *this, certificate_t *cert,
							 private_key_t *key);
	/**
	 * Clear the stored credentials.
	 */
	void (*clear)(nm_creds_t *this);

	/**
	 * Destroy a nm_creds instance.
	 */
	void (*destroy)(nm_creds_t *this);
};

/**
 * Create a nm_creds instance.
 */
nm_creds_t *nm_creds_create();

#endif /* NM_CREDS_H_ @}*/
