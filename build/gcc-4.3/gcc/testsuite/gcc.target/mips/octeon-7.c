/* Check the popcountdi2 pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64" } */
unsigned long f1 (unsigned long x) { return __builtin_popcountl (x); }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tdpop\t\\\$2,\\\$4\n" } } */
