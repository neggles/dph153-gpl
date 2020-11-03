/* Check the *branch_bit_di pattern.  */
/* { dg-mips-options "-march=octeon -Os -mabi=64" } */
int foo (void);
int f1 (unsigned long x) { if (x & (1UL << 62)) foo (); return 1; }
int f2 (unsigned long x) { if (x & (1UL << 33)) foo (); return 1; }
int f3 (unsigned long x) { if (x & (1UL << 32)) foo (); return 1; }
int f4 (unsigned long x) { if (x & (1UL << 31)) foo (); return 1; }
int f5 (unsigned long x) { if (x & (1UL << 30)) foo (); return 1; }
int f6 (unsigned long x) { if (x & (1UL << 1)) foo (); return 1; }
int f7 (unsigned long x) { if (x & (1UL << 0)) foo (); return 1; }
/* { dg-final { scan-assembler "\tbbit0\t\\\$4,62," } } */
/* { dg-final { scan-assembler "\tbbit0\t\\\$4,33," } } */
/* { dg-final { scan-assembler "\tbbit0\t\\\$4,32," } } */
/* { dg-final { scan-assembler "\tbbit0\t\\\$4,31," } } */
/* { dg-final { scan-assembler "\tbbit0\t\\\$4,30," } } */
/* { dg-final { scan-assembler "\tbbit0\t\\\$4,1," } } */
/* { dg-final { scan-assembler "\tbbit0\t\\\$4,0," } } */
