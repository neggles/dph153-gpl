/* Check the *seq_si_seq pattern.  */
/* { dg-mips-options "-march=octeon -Os -mabi=eabi -mgp32 -mno-abicalls" } */
int f1 (int x) { return x == -513; }
int f2 (int x) { return x == -512; }
int f3 (int x) { return x == -1; }
int f4 (int x) { return x == 1; }
int f5 (int x) { return x == 511; }
int f6 (int x) { return x == 512; }
int f7 (int x, int y) { return x == y; }
/* { dg-final { scan-assembler-not "\tseqi\t\[^,\]*,\[^,\]*,-513\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tseqi\t\\\$2,\\\$4,-512\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tseqi\t\\\$2,\\\$4,-1\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tseqi\t\\\$2,\\\$4,1\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tseqi\t\\\$2,\\\$4,511\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tseq\t\\\$2,\\\$4,\\\$5\n" } } */
/* { dg-final { scan-assembler-not "\tseqi\t\[^,\]*,\[^,\]*,512\n" } } */
