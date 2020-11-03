/* Check the *baddu_si pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=eabi -mgp32 -mno-abicalls -meb" } */
unsigned int
f1 (unsigned int x, unsigned int y)
{ return (unsigned char) (x + y); }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tbaddu\t\\\$2,\\\$\[45\],\\\$\[45\]\n" } } */
