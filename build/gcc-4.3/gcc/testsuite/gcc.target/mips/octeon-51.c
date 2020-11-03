/* Check the *seq_di_seq pattern.  */
/* { dg-mips-options "-march=octeon -Os -mabi=64" } */
unsigned long f1 (long x) { return x == -513; }
unsigned long f2 (long x) { return x == -512; }
unsigned long f3 (long x) { return x == -1; }
unsigned long f4 (long x) { return x == 1; }
unsigned long f5 (long x) { return x == 511; }
unsigned long f6 (long x) { return x == 512; }
unsigned long f7 (long x, long y) { return x == y; }
/* { dg-final { scan-assembler-not "\tseqi\t\[^,\]*,\[^,\]*,-513\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tseqi\t\\\$2,\\\$4,-512\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tseqi\t\\\$2,\\\$4,-1\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tseqi\t\\\$2,\\\$4,1\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tseqi\t\\\$2,\\\$4,511\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tseq\t\\\$2,\\\$4,\\\$5\n" } } */
/* { dg-final { scan-assembler-not "\tseqi\t\[^,\]*,\[^,\]*,512\n" } } */
