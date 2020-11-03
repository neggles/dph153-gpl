/* Check the *branch_bit_di pattern.  */
/* { dg-mips-options "-march=octeon -Os -mabi=64" } */
int foo (void);
int f1 (unsigned long x) { if ((x & (1UL << 62)) == 0) foo (); return 1; }
int f2 (unsigned long x) { if ((x & (1UL << 33)) == 0) foo (); return 1; }
int f3 (unsigned long x) { if ((x & (1UL << 32)) == 0) foo (); return 1; }
int f4 (unsigned long x) { if ((x & (1UL << 31)) == 0) foo (); return 1; }
int f5 (unsigned long x) { if ((x & (1UL << 30)) == 0) foo (); return 1; }
int f6 (unsigned long x) { if ((x & (1UL << 1)) == 0) foo (); return 1; }
int f7 (unsigned long x) { if ((x & (1UL << 0)) == 0) foo (); return 1; }
/* { dg-final { scan-assembler "\tbbit1\t\\\$4,62," } } */
/* { dg-final { scan-assembler "\tbbit1\t\\\$4,33," } } */
/* { dg-final { scan-assembler "\tbbit1\t\\\$4,32," } } */
/* { dg-final { scan-assembler "\tbbit1\t\\\$4,31," } } */
/* { dg-final { scan-assembler "\tbbit1\t\\\$4,30," } } */
/* { dg-final { scan-assembler "\tbbit1\t\\\$4,1," } } */
/* { dg-final { scan-assembler "\tbbit1\t\\\$4,0," } } */
