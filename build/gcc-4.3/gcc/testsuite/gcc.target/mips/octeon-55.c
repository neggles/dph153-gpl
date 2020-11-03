/* Check the *sne_si_sne_to_di pattern.  */
/* { dg-mips-options "-march=octeon -Os -mabi=64" } */
unsigned long f1 (int x) { return x != -513; }
unsigned long f2 (int x) { return x != -512; }
unsigned long f3 (int x) { return x != -1; }
unsigned long f4 (int x) { return x != 1; }
unsigned long f5 (int x) { return x != 511; }
unsigned long f6 (int x) { return x != 512; }
unsigned long f7 (int x, int y) { return x != y; }
/* { dg-final { scan-assembler-not "\tsnei\t\[^,\]*,\[^,\]*,-513\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tsnei\t\\\$2,\\\$4,-512\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tsnei\t\\\$2,\\\$4,-1\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tsnei\t\\\$2,\\\$4,1\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tsnei\t\\\$2,\\\$4,511\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tsne\t\\\$2,\\\$4,\\\$5\n" } } */
/* { dg-final { scan-assembler-not "\tsnei\t\[^,\]*,\[^,\]*,512\n" } } */
