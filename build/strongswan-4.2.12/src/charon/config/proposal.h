/*
 * Copyright (C) 2006 Martin Willi
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
 * $Id: proposal.h 4062 2008-06-12 11:42:19Z martin $
 */

/**
 * @defgroup proposal proposal
 * @{ @ingroup config
 */

#ifndef PROPOSAL_H_
#define PROPOSAL_H_

typedef enum protocol_id_t protocol_id_t;
typedef enum transform_type_t transform_type_t;
typedef enum extended_sequence_numbers_t extended_sequence_numbers_t;
typedef struct proposal_t proposal_t;

#include <library.h>
#include <utils/identification.h>
#include <utils/linked_list.h>
#include <utils/host.h>
#include <crypto/crypters/crypter.h>
#include <crypto/signers/signer.h>
#include <crypto/diffie_hellman.h>
#include <config/traffic_selector.h>

/**
 * Protocol ID of a proposal.
 */
enum protocol_id_t {
	PROTO_NONE = 0,
	PROTO_IKE = 1,
	PROTO_AH = 2,
	PROTO_ESP = 3,
};

/**
 * enum names for protocol_id_t
 */
extern enum_name_t *protocol_id_names;


/**
 * Type of a transform, as in IKEv2 RFC 3.3.2.
 */
enum transform_type_t {
	UNDEFINED_TRANSFORM_TYPE = 241,
	ENCRYPTION_ALGORITHM = 1,
	PSEUDO_RANDOM_FUNCTION = 2,
	INTEGRITY_ALGORITHM = 3,
	DIFFIE_HELLMAN_GROUP = 4,
	EXTENDED_SEQUENCE_NUMBERS = 5
};

/**
 * enum names for transform_type_t.
 */
extern enum_name_t *transform_type_names;


/**
 * Extended sequence numbers, as in IKEv2 RFC 3.3.2.
 */
enum extended_sequence_numbers_t {
	NO_EXT_SEQ_NUMBERS = 0,
	EXT_SEQ_NUMBERS = 1
};

/**
 * enum strings for extended_sequence_numbers_t.
 */
extern enum_name_t *extended_sequence_numbers_names;

/**
 * Stores a set of algorithms used for an SA.
 * 
 * A proposal stores algorithms for a specific 
 * protocol. It can store algorithms for one protocol.
 * Proposals with multiple protocols are not supported,
 * as it's not specified in RFC4301 anymore.
 */
struct proposal_t {
	
	/**
	 * Add an algorithm to the proposal.
	 * 
	 * The algorithms are stored by priority, first added
	 * is the most preferred.
	 * Key size is only needed for encryption algorithms
	 * with variable key size (such as AES). Must be set
	 * to zero if key size is not specified.
	 * The alg parameter accepts encryption_algorithm_t,
	 * integrity_algorithm_t, dh_group_number_t and
	 * extended_sequence_numbers_t.
	 * 
	 * @param type			kind of algorithm
	 * @param alg			identifier for algorithm
	 * @param key_size		key size to use
	 */
	void (*add_algorithm) (proposal_t *this, transform_type_t type,
						   u_int16_t alg, u_int16_t key_size);
	
	/**
	 * Get an enumerator over algorithms for a specifc algo type.
	 * 
	 * @param type			kind of algorithm
	 * @return				enumerator over u_int16_t alg, u_int16_t key_size
	 */
	enumerator_t *(*create_enumerator) (proposal_t *this, transform_type_t type);
	
	/**
	 * Get the algorithm for a type to use.
	 * 
	 * If there are multiple algorithms, only the first is returned.
	 * 
	 * @param type			kind of algorithm
	 * @param alg			pointer which receives algorithm
	 * @param key_size		pointer which receives the key size
	 * @return				TRUE if algorithm of this kind available
	 */
	bool (*get_algorithm) (proposal_t *this, transform_type_t type,
						   u_int16_t *alg, u_int16_t *key_size);
	
	/**
	 * Check if the proposal has a specific DH group.
	 * 
	 * @param group			group to check for
	 * @return				TRUE if algorithm included
	 */
	bool (*has_dh_group) (proposal_t *this, diffie_hellman_group_t group);
	
	/**
	 * Strip DH groups from proposal to use it without PFS.
	 */
	void (*strip_dh)(proposal_t *this);	

	/**
	 * Compare two proposal, and select a matching subset.
	 * 
	 * If the proposals are for the same protocols (AH/ESP), they are
	 * compared. If they have at least one algorithm of each type
	 * in common, a resulting proposal of this kind is created.
	 * 
	 * @param other			proposal to compair agains
	 * @return				selected proposal, NULL if proposals don't match
	 */
	proposal_t *(*select) (proposal_t *this, proposal_t *other);
	
	/**
	 * Get the protocol ID of the proposal.
	 *
	 * @return				protocol of the proposal
	 */
	protocol_id_t (*get_protocol) (proposal_t *this);
	
	/**
	 * Get the SPI of the proposal.
	 * 
	 * @return				spi for proto
	 */
	u_int64_t (*get_spi) (proposal_t *this);
	
	/**
	 * Set the SPI of the proposal.
	 * 
	 * @param spi			spi to set for proto
	 */
	void (*set_spi) (proposal_t *this, u_int64_t spi);
	
	/**
	 * Check for the eqality of two proposals.
	 *
	 * @param other			other proposal to check for equality
	 * @return				TRUE if other equal to this
	 */
	bool (*equals)(proposal_t *this, proposal_t *other);
	
	/**
	 * Clone a proposal.
	 * 
	 * @return				clone of proposal
	 */
	proposal_t *(*clone) (proposal_t *this);
	
	/**
	 * Destroys the proposal object.
	 */
	void (*destroy) (proposal_t *this);
};

/**
 * Create a child proposal for AH, ESP or IKE.
 *
 * @param protocol			protocol, such as PROTO_ESP
 * @return 					proposal_t object
 */
proposal_t *proposal_create(protocol_id_t protocol);

/**
 * Create a default proposal if nothing further specified.
 *
 * @param protocol			protocol, such as PROTO_ESP
 * @return 					proposal_t object
 */
proposal_t *proposal_create_default(protocol_id_t protocol);

/**
 * Create a proposal from a string identifying the algorithms.
 *
 * The string is in the same form as a in the ipsec.conf file.
 * E.g.:	aes128-sha2_256-modp2048
 *          3des-md5
 * An additional '!' at the end of the string forces this proposal,
 * without it the peer may choose another algorithm we support.
 *
 * @param protocol			protocol, such as PROTO_ESP
 * @param algs				algorithms as string
 * @return 					proposal_t object
 */
proposal_t *proposal_create_from_string(protocol_id_t protocol, const char *algs);

/**
 * Get printf hooks for a proposal.
 *
 * Arguments are: 
 *    proposal_t *proposal
 * With the #-specifier, arguments are:
 *    linked_list_t *list containing proposal_t*
 */
printf_hook_functions_t proposal_get_printf_hooks();

#endif /* PROPOSAL_H_ @} */
