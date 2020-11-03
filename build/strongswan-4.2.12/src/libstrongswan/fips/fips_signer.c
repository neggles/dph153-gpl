/*
 * Copyright (C) 2007 Bruno Krieg, Daniel Wydler
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
 * $Id: fips_signer.c 4258 2008-08-19 18:51:30Z andreas $
 */

#include <stdio.h>

#include <crypto/hashers/hasher.h>
#include "fips.h"

int main(int argc, char* argv[])
{
	FILE *f;
	char *hmac_key = "strongSwan Version " VERSION;
	char  hmac_signature[BUF_LEN];

	/* initialize library */
	library_init(STRONGSWAN_CONF);
#ifdef USE_SHA1
	lib->plugins->load(lib->plugins, PLUGINDIR "/sha1/.libs", "sha1");
#endif
#ifdef USE_OPENSSL
	lib->plugins->load(lib->plugins, PLUGINDIR "/openssl/.libs", "openssl");
#endif
	lib->plugins->load(lib->plugins, PLUGINDIR "/hmac/.libs", "hmac");

	if (!fips_compute_hmac_signature(hmac_key, hmac_signature))
	{
		exit(1);
	}
	
	/**
     * write computed HMAC signature to fips_signature.h
	 */
	f = fopen("fips_signature.h", "wt");

	if (f == NULL)
	{
		exit(1);
	}
	fprintf(f, "/* SHA-1 HMAC signature computed over TEXT and RODATA of libstrongswan\n");
	fprintf(f, " *\n");
	fprintf(f, " * This file has been automatically generated by fips_signer\n");
	fprintf(f, " * Do not edit manually!\n");
	fprintf(f, " */\n");
	fprintf(f, "\n");
	fprintf(f, "#ifndef FIPS_SIGNATURE_H_\n");
	fprintf(f, "#define FIPS_SIGNATURE_H_\n");
	fprintf(f, "\n");
	fprintf(f, "const char *hmac_key = \"%s\";\n", hmac_key);
	fprintf(f, "const char *hmac_signature = \"%s\";\n", hmac_signature);
	fprintf(f, "\n");
	fprintf(f, "#endif /* FIPS_SIGNATURE_H_ @} */\n");
	fclose(f);

	library_deinit();
	exit(0);
}
