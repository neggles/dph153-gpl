/* Check the *trunc_zero_ext_qidi pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64" } */
unsigned long f1 (unsigned long x) { return (unsigned char) x; }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tandi\t\\\$2,\\\$4,0x00ff\n" } } */
