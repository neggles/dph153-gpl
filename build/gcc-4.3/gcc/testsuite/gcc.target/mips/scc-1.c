/* Check the *seq_si_to_di pattern.  */
/* { dg-mips-options "-march=mips64 -Os -mabi=64" } */
unsigned long f (unsigned int x) { return x == 0; }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tsltu\t\\\$2,\\\$4,1\n" } } */
