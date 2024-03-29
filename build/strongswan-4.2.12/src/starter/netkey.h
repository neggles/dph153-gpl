/* strongSwan netkey initialization and cleanup 
 * Copyright (C) 2001-2002 Mathieu Lafon - Arkoon Network Security
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
 * RCSID $Id: netkey.h 3267 2007-10-08 19:57:54Z andreas $
 */

#ifndef _STARTER_NETKEY_H_
#define _STARTER_NETKEY_H_

extern bool starter_netkey_init (void);
extern void starter_netkey_cleanup (void);

#endif /* _STARTER_NETKEY_H_ */

