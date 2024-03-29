/*
 * Copyright (C) 2007-2008 Martin Willi
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
 */

/**
 * @defgroup certificate certificate
 * @{ @ingroup certificates
 */

#ifndef CERTIFICATE_H_
#define CERTIFICATE_H_

typedef struct certificate_t certificate_t;
typedef enum certificate_type_t certificate_type_t;
typedef enum cert_validation_t cert_validation_t;

#include <library.h>
#include <utils/identification.h>
#include <credentials/keys/public_key.h>

/**
 * Kind of a certificate_t
 */
enum certificate_type_t {
	/** just any certificate */
	CERT_ANY,
	/** X.509 certificate */
	CERT_X509,
	/** X.509 certificate revocation list */
	CERT_X509_CRL,
	/** X.509 online certificate status protocol request */
	CERT_X509_OCSP_REQUEST,
	/** X.509 online certificate status protocol response */
	CERT_X509_OCSP_RESPONSE,
	/** X.509 attribute certificate */
	CERT_X509_AC,
	/** trusted, preinstalled public key */
	CERT_TRUSTED_PUBKEY,
	/** PGP certificate */
	CERT_PGP,
};

/**
 * Enum names for certificate_type_t
 */
extern enum_name_t *certificate_type_names;

/**
 * Result of a certificate validation.
 */
enum cert_validation_t {
	/** certificate has been validated successfully */
	VALIDATION_GOOD,
	/** certificate has been validated, but check based on stale information */
	VALIDATION_STALE,
	/** certificate has been revoked */
	VALIDATION_REVOKED,
	/** validation failed due to a processing error */
	VALIDATION_FAILED,
	/** validation has been skipped due to missing validation information */
	VALIDATION_SKIPPED,
};

/**
 * Enum names for cert_validation_t
 */
extern enum_name_t *cert_validation_names;

/**
 * An abstract certificate.
 *
 * A certificate designs a subject-issuer relationship. It may have an 
 * associated public key.
 */
struct certificate_t {

	/**
	 * Get the type of the certificate.
	 *
	 * @return			certifcate type
	 */
	certificate_type_t (*get_type)(certificate_t *this);

	/**
	 * Get the primary subject to which this certificate belongs.
	 *
	 * @return			subject identity
	 */
	identification_t* (*get_subject)(certificate_t *this);
	
	/**
	 * Check if certificate contains a subject ID.
	 *
	 * A certificate may contain additional subject identifiers, which are
	 * not returned by get_subject (e.g. subjectAltNames)
	 *
	 * @param subject	subject identity
	 * @return 			matching value of best match
	 */
	id_match_t (*has_subject)(certificate_t *this, identification_t *subject);
		
	/**
	 * Get the issuer which signed this certificate.
	 *
	 * @return			issuer identity
	 */
	identification_t* (*get_issuer)(certificate_t *this);
	
	/**
	 * Check if certificate contains an issuer ID.
	 *
	 * A certificate may contain additional issuer identifiers, which are
	 * not returned by get_issuer (e.g. issuerAltNames)
	 *
	 * @param subject	isser identity
	 * @return 			matching value of best match
	 */
	id_match_t (*has_issuer)(certificate_t *this, identification_t *issuer);
	
	/**
	 * Check if this certificate is issued and signed by a specific issuer.
	 *
	 * @param issuer	issuer's certificate
	 * @return 			TRUE if certificate issued by issuer and trusted
	 */
	bool (*issued_by)(certificate_t *this, certificate_t *issuer);
	
	/**
	 * Get the public key associated to this certificate.
	 *
	 * @return			newly referenced public_key, NULL if none available
	 */
	public_key_t* (*get_public_key)(certificate_t *this);
	
	/**
	 * Check the lifetime of the certificate.
	 *
	 * @param when			check validity at a certain time (NULL for now)
	 * @param not_before	receives certificates start of lifetime
	 * @param not_after		receives certificates end of lifetime
	 * @return				TRUE if when between not_after and not_before
	 */
	bool (*get_validity)(certificate_t *this, time_t *when,
						 time_t *not_before, time_t *not_after);
	
	/**
	 * Is this newer than that?
	 *
	 * @return			TRUE if newer, FALSE otherwise
	 */
	bool (*is_newer)(certificate_t *this, certificate_t *that);
	
	/**
	 * Get the certificate in an encoded form.
	 *
	 * @return				allocated chunk of encoded cert
	 */
	chunk_t (*get_encoding)(certificate_t *this);
	
	/**
	 * Check if two certificates are equal.
	 *
	 * @param other			certificate to compair against this
	 * @return				TRUE if certificates are equal
	 */
	bool (*equals)(certificate_t *this, certificate_t *other);
	
	/**
	 * Get a new reference to the certificate.
	 *
	 * @return			this, with an increased refcount 
	 */
	certificate_t* (*get_ref)(certificate_t *this);
	
	/**
     * Destroy a certificate.
     */
    void (*destroy)(certificate_t *this);
};

#endif /* CERTIFICATE_H_ @}*/
