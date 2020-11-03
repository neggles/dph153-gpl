/* Check the *baddu_disi pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64 -meb" } */
unsigned int f1 (unsigned long x, unsigned long y) { return (x + y) & 0xff; }
/* At the time of writing, we have an unncessary sign-extension of the
   return reigster, so f1() is not a two-insn function.  */
/* { dg-final { scan-assembler "\tbaddu\t" } } */
