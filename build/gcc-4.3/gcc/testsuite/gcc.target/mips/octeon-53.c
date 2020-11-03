/* Check the *sne_si_sne pattern.  */
/* { dg-mips-options "-march=octeon -Os -mabi=64" } */
void f1 (int x, int *p) { *p = x != -513; }
void f2 (int x, int *p) { *p = x != -512; }
void f3 (int x, int *p) { *p = x != -1; }
void f4 (int x, int *p) { *p = x != 1; }
void f5 (int x, int *p) { *p = x != 511; }
void f6 (int x, int *p) { *p = x != 512; }
void f7 (int x, int y, int *p) { *p = x != y; }
/* { dg-final { scan-assembler-not "\tsnei\t\[^,\]*,\[^,\]*,-513\n" } } */
/* { dg-final { scan-assembler "\tsnei\t\[^,\]*,\\\$4,-512\n" } } */
/* { dg-final { scan-assembler "\tsnei\t\[^,\]*,\\\$4,-1\n" } } */
/* { dg-final { scan-assembler "\tsnei\t\[^,\]*,\\\$4,1\n" } } */
/* { dg-final { scan-assembler "\tsnei\t\[^,\]*,\\\$4,511\n" } } */
/* { dg-final { scan-assembler "\tsne\t\[^,\]*,\\\$4,\\\$5\n" } } */
/* { dg-final { scan-assembler-not "\tsnei\t\[^,\]*,\[^,\]*,512\n" } } */
