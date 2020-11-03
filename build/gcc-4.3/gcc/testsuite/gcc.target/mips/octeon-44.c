/* Check the *branch_bit pattern.  */
/* { dg-mips-options "-march=octeon -Os -mabi=eabi -mgp32 -mno-abicalls" } */
int foo (void);
int f1 (unsigned int x) { if ((x & (1UL << 30)) == 0) foo (); return 1; }
int f2 (unsigned int x) { if ((x & (1UL << 1)) == 0) foo (); return 1; }
int f3 (unsigned int x) { if ((x & (1UL << 0)) == 0) foo (); return 1; }
/* { dg-final { scan-assembler "\tbbit1\t\\\$4,30," } } */
/* { dg-final { scan-assembler "\tbbit1\t\\\$4,1," } } */
/* { dg-final { scan-assembler "\tbbit1\t\\\$4,0," } } */
