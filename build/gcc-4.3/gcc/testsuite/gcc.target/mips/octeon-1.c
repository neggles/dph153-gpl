/* Check the *baddu_si pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=eabi -mgp32 -meb -mno-abicalls" } */
unsigned int f1 (unsigned int x, unsigned int y) { return (x + y) & 0xff; }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tbaddu\t\\\$2,\\\$\[45\],\\\$\[45\]\n" } } */
