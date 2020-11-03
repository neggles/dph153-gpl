/*
 * Copyright (C) 2008 Free Software Foundation, Inc.
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3, or (at your option) any
 * later version.
 * 
 * In addition to the permissions in the GNU General Public License, the
 * Free Software Foundation gives you unlimited permission to link the
 * compiled version of this file with other programs, and to distribute
 * those programs without any restriction coming from the use of this
 * file.  (The General Public License restrictions do apply in other
 * respects; for example, they cover modification of the file, and
 * distribution when not linked into another program.)
 * 
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GCC; see the file COPYING3.  If not see
 * <http://www.gnu.org/licenses/>.
 * 
 *    As a special exception, if you link this library with files
 *    compiled with GCC to produce an executable, this does not cause
 *    the resulting executable to be covered by the GNU General Public License.
 *    This exception does not however invalidate any other reasons why
 *    the executable file might be covered by the GNU General Public License.
 */

#ifdef __mips_hard_float

/* flush denormalized numbers to zero */
#define _FPU_FLUSH_TZ   0x1000000

/* rounding control */
#define _FPU_RC_NEAREST 0x0     /* RECOMMENDED */
#define _FPU_RC_ZERO    0x1
#define _FPU_RC_UP      0x2
#define _FPU_RC_DOWN    0x3

/* enable interrupts for IEEE exceptions */
#define _FPU_IEEE     0x00000F80

/* Macros for accessing the hardware control word.  */
#define _FPU_GETCW(cw) __asm__ ("cfc1 %0,$31" : "=r" (cw))
#define _FPU_SETCW(cw) __asm__ ("ctc1 %0,$31" : : "r" (cw))

static void __attribute__((constructor))
set_fast_math (void)
{
  unsigned int fcr;

  /* fastmath: flush to zero, round to nearest, ieee exceptions disabled */
  fcr = _FPU_FLUSH_TZ | _FPU_RC_NEAREST;

  _FPU_SETCW(fcr);
}

#endif /* __mips_hard_float */
