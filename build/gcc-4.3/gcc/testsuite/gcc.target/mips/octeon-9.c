/* Check the *lshiftrt_trunc_exts pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64" } */
unsigned int f1 (unsigned long x) { return x >> 7; }
/* At the time of writing, we have an unncessary sign-extension of the
   return reigster, so f1() is not a two-insn function.  */
/* { dg-final { scan-assembler "\texts\t\[^,\]*,\[^,\]*,7,31" } } */
