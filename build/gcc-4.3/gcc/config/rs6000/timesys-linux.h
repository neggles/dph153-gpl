/* Configuration file for timesys ARM GNU/Linux EABI targets.
   Copyright (C) 2007
   Free Software Foundation, Inc.
   Contributed by CodeSourcery, LLC   

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 3, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

/* Add -t flags for convenience in generating multilibs.  */
#undef CC1_EXTRA_SPEC
#define CC1_EXTRA_SPEC \
  "%{te500v1: -mcpu=8540 -mfloat-gprs=single -mspe=yes -mabi=spe} "	\
  "%{te600: -mcpu=7400 -maltivec -mabi=altivec} "

#undef ASM_DEFAULT_SPEC
#define ASM_DEFAULT_SPEC			\
  "%{te500v1:-mppc -mspe -me500 ;		\
     te600:-mppc -maltivec ;			\
     mcpu=405:-m405 ;				\
     mcpu=440:-m440 ;				\
     :-mppc%{m64:64}}"


/* FIXME:We should be dynamically creating this from the makefile.
   See m68k for an example.  */
#undef SYSROOT_SUFFIX_SPEC
#define SYSROOT_SUFFIX_SPEC \
  "%{mcpu=405:/4xx ; mcpu=440:/44x ; mcpu=801:/8xx ; te500v1:/85xx ; te600:/74xx}"
