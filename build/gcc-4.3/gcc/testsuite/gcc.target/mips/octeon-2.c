/* Check the *baddu_didi pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64 -meb" } */
unsigned long f1 (unsigned int x, unsigned int y) { return (x + y) & 0xff; }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tbaddu\t\\\$2,\\\$\[45\],\\\$\[45\]\n" } } */
