/*
 * Copyright (C) 2008 Andreas Steffen
 * Hochschule fuer Technik Rapperswil, Switzerland
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
 * $Id: pkcs9.h 3589 2008-03-13 14:14:44Z martin $
 */
 
/**
 * @defgroup pkcs9 pkcs9
 * @{ @ingroup crypto
 */

#ifndef PKCS9_H_
#define PKCS9_H_

typedef struct pkcs9_t pkcs9_t;

#include <library.h>

/**
 * PKCS#9 attributes.
 */
struct pkcs9_t {
	
	/**
	 * Generate ASN.1 encoding of attribute list
	 */
	void (*build_encoding) (pkcs9_t *this);

	/**
	 * Gets ASN.1 encoding of PKCS#9 attribute list
	 *
	 * @return				ASN.1 encoded PKCSI#9 list
	 */
	chunk_t (*get_encoding) (pkcs9_t *this);

	/**
	 * Gets a PKCS#9 attribute
	 *
	 * @param oid			OID of the attribute
	 * @return				ASN.1 encoded value of the attribute
	 */
	chunk_t (*get_attribute) (pkcs9_t *this, int oid);

	/**
	 * Adds a PKCS#9 attribute
	 *
	 * @param oid			OID of the attribute
	 * @param value			ASN.1 encoded value of the attribute 
	 */
	void (*set_attribute) (pkcs9_t *this, int oid, chunk_t value);

	/**
	 * Gets a PKCS#9 messageDigest attribute
	 *
	 * @return				messageDigest
	 */
	chunk_t (*get_messageDigest) (pkcs9_t *this);

	/**
	 * Add a PKCS#9 messageDigest attribute
	 *
	 * @param value			messageDigest 
	 */
	void (*set_messageDigest) (pkcs9_t *this, chunk_t value);

	/**
	 * Destroys the PKCS#9 attribute list.
	 */
	void (*destroy) (pkcs9_t *this);
};

/**
 * Read a PKCS#9 attribute list from a DER encoded chunk.
 * 
 * @param chunk		chunk containing DER encoded data
 * @param level		ASN.1 parsing start level
 * @return 			created pkcs9 attribute list, or NULL if invalid.
 */
pkcs9_t *pkcs9_create_from_chunk(chunk_t chunk, u_int level);

/**
 * Create an empty PKCS#9 attribute list
 * 
 * @param chunk			chunk containing data
 * @return 				created pkcs9 attribute list.
 */
pkcs9_t *pkcs9_create(void);

#endif /* PKCS9_H_ @} */
