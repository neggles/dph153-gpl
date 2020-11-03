/* Check the popcountsi2 pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=eabi -mgp32 -mno-abicalls" } */
unsigned int f1 (unsigned int x) { return __builtin_popcount (x); }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tpop\t\\\$2,\\\$4\n" } } */
