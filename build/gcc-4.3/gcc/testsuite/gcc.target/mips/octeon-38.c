/* Check the *truncsi_store[qh]i patterns.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64" } */
void
f1 (unsigned char *dest, unsigned long x)
{ unsigned int y = x; dest[41] = y; }
void
f2 (unsigned short *dest, unsigned long x)
{ unsigned int y = x; dest[101] = y; }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tsb\t\\\$5,41\\(\\\$4\\)\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tsh\t\\\$5,202\\(\\\$4\\)\n" } } */
