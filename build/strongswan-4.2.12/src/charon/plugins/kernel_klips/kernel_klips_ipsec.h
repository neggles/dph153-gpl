/*
 * Copyright (C) 2008 Tobias Brunner
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
 * $Id: kernel_klips_ipsec.h 4617 2008-11-11 08:45:19Z tobias $
 */

/**
 * @defgroup kernel_klips_ipsec_i kernel_klips_ipsec
 * @{ @ingroup kernel_klips
 */

#ifndef KERNEL_KLIPS_IPSEC_H_
#define KERNEL_KLIPS_IPSEC_H_

#include <kernel/kernel_ipsec.h>

typedef struct kernel_klips_ipsec_t kernel_klips_ipsec_t;

/**
 * Implementation of the kernel ipsec interface using PF_KEY.
 */
struct kernel_klips_ipsec_t {

	/**
	 * Implements kernel_ipsec_t interface
	 */
	kernel_ipsec_t interface;
};

/**
 * Create a PF_KEY kernel ipsec interface instance.
 *
 * @return			kernel_klips_ipsec_t instance
 */
kernel_klips_ipsec_t *kernel_klips_ipsec_create();

#endif /* KERNEL_KLIPS_IPSEC_H_ @} */
