/* Test the `vreinterpretQf32_u16' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0 -mfpu=neon -mfloat-abi=softfp" } */

#include "arm_neon.h"

void test_vreinterpretQf32_u16 (void)
{
  float32x4_t out_float32x4_t;
  uint16x8_t arg0_uint16x8_t;

  out_float32x4_t = vreinterpretq_f32_u16 (arg0_uint16x8_t);
}

/* { dg-final { cleanup-saved-temps } } */
