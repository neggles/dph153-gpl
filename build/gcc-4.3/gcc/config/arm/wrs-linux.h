/* Wind River GNU/Linux Configuration.
   Copyright (C) 2006, 2007, 2008
   Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

/* Use the ARM926EJ-S by default.  */
#undef SUBTARGET_CPU_DEFAULT
#define SUBTARGET_CPU_DEFAULT TARGET_CPU_arm926ejs

/* Add a -tiwmmxt option for convenience in generating multilibs.
   This option generates big-endian IWMMXT code.  */
#undef CC1_SPEC
#define CC1_SPEC "							\
 %{tarm926ej-s:	-mcpu=arm926ej-s ;					\
   tiwmmxt:     -mcpu=iwmmxt ;						\
   tiwmmxt2:    -mcpu=iwmmxt ;						\
   txscale:     -mcpu=xscale -mbig-endian ;				\
   tarm920t:    -mcpu=arm920t ;						\
   tthumb2:     %{!mcpu=*:%{!march=*:-march=armv6t2}} -mthumb ;		\
   tcortex-a8-be8: -mcpu=cortex-a8 -mbig-endian -mfloat-abi=softfp	\
                   -mfpu=neon }						\
 %{txscale:%{mfloat-abi=softfp:%eXScale VFP multilib not provided}}	\
 %{tarm920t:%{mfloat-abi=softfp:%eARM920T VFP multilib not provided}}	\
 %{profile:-p}"

/* Since the ARM926EJ-S is the default processor, we do not need to
   provide an explicit multilib for that processor.  */
#undef MULTILIB_DEFAULTS
#define MULTILIB_DEFAULTS \
  { "tarm926ej-s" }

/* The GLIBC headers are in /usr/include, relative to the sysroot; the
   uClibc headers are in /uclibc/usr/include.  */
#undef SYSROOT_HEADERS_SUFFIX_SPEC
#define SYSROOT_HEADERS_SUFFIX_SPEC		\
  "%{muclibc:/uclibc}" 

/* Translate -tiwmmxt appropriately for the assembler.  The -meabi=5
   option is the relevant part of SUBTARGET_EXTRA_ASM_SPEC in bpabi.h.  */
#undef SUBTARGET_EXTRA_ASM_SPEC
#define SUBTARGET_EXTRA_ASM_SPEC \
  "%{tiwmmxt2:-mcpu=iwmmxt2} %{tiwmmxt:-mcpu=iwmmxt} %{txscale:-mcpu=xscale -EB} %{tcortex-a8-be8:-mcpu=cortex-a8 -EB} -meabi=5" 

/* Translate -tiwmmxt for the linker.  */
#undef SUBTARGET_EXTRA_LINK_SPEC
#define SUBTARGET_EXTRA_LINK_SPEC			\
  " %{tiwmmxt:-m armelf_linux_eabi ;			\
     txscale:-m armelfb_linux_eabi ;			\
     tcortex-a8-be8:-m armelfb_linux_eabi %{!r:--be8} ;	\
     : -m armelf_linux_eabi}"

/* The various C libraries each have their own subdirectory.  */
#undef SYSROOT_SUFFIX_SPEC
#define SYSROOT_SUFFIX_SPEC					\
  "%{muclibc:/uclibc}%{tiwmmxt:/tiwmmxt ;			\
     tiwmmxt2:/tiwmmxt ;					\
     txscale:/txscale ;						\
     tarm920t:/tarm920t ;					\
     tthumb2:/thumb2 ;						\
     tcortex-a8-be8:/cortex-a8-be8}%{!tthumb2:%{!tcortex-a8-be8:%{mfloat-abi=softfp:/softfp}}}"

