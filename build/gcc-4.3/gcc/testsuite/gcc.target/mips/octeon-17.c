/* Check the *extendqidi2_hw pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64" } */
unsigned long f1 (unsigned int x) { return (signed char) x; }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tseb\t\\\$2,\\\$4\n" } } */
