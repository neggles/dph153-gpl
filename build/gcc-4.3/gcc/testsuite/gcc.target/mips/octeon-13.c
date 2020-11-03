/* Check the *zero_extendsidi2_dext pattern (memory alternative).  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64" } */
unsigned long f1 (unsigned int *x) { return x[4]; }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tlwu\t\\\$2,16\\(\\\$4\\)\n" } } */
