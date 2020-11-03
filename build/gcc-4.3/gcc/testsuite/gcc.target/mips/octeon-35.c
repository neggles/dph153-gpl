/* Check the *cins pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64" } */
unsigned long f1 (unsigned long x) { return (x & 0xfff) << 4; }
unsigned long f2 (unsigned long x) { return (x & 0x3ff) << 40; }
unsigned long f3 (unsigned long x) { return (x & 0xffffffff) << 2; }
unsigned long f4 (unsigned long x) { return (x & 0x7ff) << 58; }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tcins\t\\\$2,\\\$4,4,11\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tcins\t\\\$2,\\\$4,40,9\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tcins\t\\\$2,\\\$4,2,31\n" } } */
/* { dg-final { scan-assembler-not "cins\t\[^,\]*,\[^,\]*,58,10\n" } } */
