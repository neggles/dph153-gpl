/* Check the *clear_upper32_dext pattern (register alternative).  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64" } */
unsigned long
f1 (unsigned long x)
{ unsigned long mask = 0xffffffffUL; return x & mask; }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tdext\t\\\$2,\[^,\]*,0,32\n" } } */
