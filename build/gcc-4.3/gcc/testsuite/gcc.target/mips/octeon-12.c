/* Check the *zero_extendsidi2_dext pattern (register alternative).  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64" } */
unsigned long f1 (unsigned int x) { return x; }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tdext\t\\\$2,\\\$4,0,32\n" } } */
