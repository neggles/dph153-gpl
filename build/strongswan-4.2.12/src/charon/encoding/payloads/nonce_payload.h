/*
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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
 * $Id: nonce_payload.h 4819 2008-12-31 08:58:49Z martin $
 */

/**
 * @defgroup nonce_payload nonce_payload
 * @{ @ingroup payloads
 */

#ifndef NONCE_PAYLOAD_H_
#define NONCE_PAYLOAD_H_

typedef struct nonce_payload_t nonce_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>

/**
 * Nonce size in bytes for nonces sending to other peer.
 */
#define NONCE_SIZE 32

/**
 * Length of a nonce payload without a nonce in bytes.
 */
#define NONCE_PAYLOAD_HEADER_LENGTH 4

/**
 * Object representing an IKEv2 Nonce payload.
 * 
 * The Nonce payload format is described in RFC section 3.3.
 */
struct nonce_payload_t {
	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Set the nonce value.
	 *
	 * @param nonce	  		chunk containing the nonce, will be cloned
	 */
	void (*set_nonce) (nonce_payload_t *this, chunk_t nonce);
	
	/**
	 * Get the nonce value.
	 *
	 * @return				a chunk containing the cloned nonce
	 */
	chunk_t (*get_nonce) (nonce_payload_t *this);
	
	/**
	 * Destroys an nonce_payload_t object.
	 */
	void (*destroy) (nonce_payload_t *this);
};

/**
 * Creates an empty nonce_payload_t object
 * 
 * @return nonce_payload_t object
 */
nonce_payload_t *nonce_payload_create(void);

#endif /*NONCE_PAYLOAD_H_ @} */
