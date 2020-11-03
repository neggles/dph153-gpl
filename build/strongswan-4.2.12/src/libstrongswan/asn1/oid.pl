#!/usr/bin/perl
# Generates oid.h and oid.c out of oid.txt
#
# Copyright (C) 2003-2008 Andreas Steffen
# Hochschule fuer Technik Rapperswil
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#

$copyright="Copyright (C) 2003-2008 Andreas Steffen, Hochschule fuer Technik Rapperswil";
$automatic="This file has been automatically generated by the script oid.pl";
$warning="Do not edit manually!";

print "oid.pl generating oid.h and oid.c\n";

# Generate oid.h

open(OID_H,  ">oid.h")
    or die "could not open 'oid.h': $!";

print OID_H "/* Object identifiers (OIDs) used by strongSwan\n",
	    " * ", $copyright, "\n",
	    " * \n",
	    " * ", $automatic, "\n",
	    " * ", $warning, "\n",
	    " */\n\n",
	    "#ifndef OID_H_\n",
	    "#define OID_H_\n\n",
	    "typedef struct {\n",
	    "    u_char octet;\n",
	    "    u_int  next;\n",
	    "    u_int  down;\n",
	    "    const u_char *name;\n",
	    "} oid_t;\n",
	    "\n",
            "extern const oid_t oid_names[];\n",
	    "\n",
	    "#define OID_UNKNOWN							-1\n";

# parse oid.txt

open(SRC,  "<oid.txt")
    or die "could not open 'oid.txt': $!";

$counter = 0;
$max_name = 0;
$max_order = 0;

while ($line = <SRC>)
{
    $line =~ m/( *?)(0x\w{2})\s+(".*?")[ \t]*?([\w_]*?)\Z/;

    @order[$counter] = length($1);
    @octet[$counter] = $2;
    @name[$counter] = $3;

    if (length($1) > $max_order)
    {
	$max_order = length($1);
    }
    if (length($3) > $max_name)
    {
	$max_name = length($3);
    }
    if (length($4) > 0)
    {
	printf OID_H "#define %s%s%d\n", $4, "\t" x ((39-length($4))/4), $counter;
    }
    $counter++;
}

print OID_H "\n#endif /* OID_H_ */\n";

close SRC;
close OID_H;

# Generate oid.c

open(OID_C, ">oid.c")
    or die "could not open 'oid.c': $!";

print OID_C "/* List of some useful object identifiers (OIDs)\n",
            " * ", $copyright, "\n",
	    " * \n",
	    " * ", $automatic, "\n",
	    " * ", $warning, "\n",
	    " */\n",
	    "\n",
	    "#include <stdlib.h>\n",
	    "\n",
	    "#include \"oid.h\"\n",
	    "\n",
            "const oid_t oid_names[] = {\n";

for ($c = 0; $c < $counter; $c++)
{
    $next = 0;

    for ($d = $c+1; $d < $counter && @order[$d] >= @order[$c]; $d++)
    {
	if (@order[$d] == @order[$c])
	{
	    @next[$c] = $d;
	    last;
	}
    }

    printf OID_C "  {%s%s,%s%3d, %d, %s%s}%s  /* %3d */\n"
	,' '  x @order[$c]
	, @octet[$c]
	, ' ' x (1 + $max_order - @order[$c])
	, @next[$c]
	, @order[$c+1] > @order[$c]
	, @name[$c]
	, ' ' x ($max_name - length(@name[$c]))
	, $c != $counter-1 ? "," : " "
	, $c;
}

print OID_C "};\n" ;
close OID_C;