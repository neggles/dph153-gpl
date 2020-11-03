/* Check the *clear_upper32_dext pattern (memory alternative).  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64" } */
unsigned long f1 (unsigned long *x) { return x[4] & 0xffffffffUL; }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tlwu\t\\\$2,..\\(\\\$4\\)\n" } } */
