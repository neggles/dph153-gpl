/* Verify that hardware multiply is preferred on XScale. */
/* { dg-do compile } */
/* { dg-options "-mcpu=xscale -O2" } */
/* { dg-skip-if "" { *-*-* } { "-march=*" } { "-march=xscale" } } */
/* { dg-require-effective-target arm32 } */

/* Brett Gaines' test case. */
unsigned BCPL(unsigned) __attribute__ ((naked));
unsigned BCPL(unsigned seed)
{
    /* Best code would be:
       ldr r1, =2147001325
       ldr r2, =715136305
       mla r0, r1, r0, r2
       mov pc, lr */

    return seed * 2147001325U + 715136305U;
}

/* { dg-final { scan-assembler "mla\[ 	].*" } } */
