/* Check the *extz_truncdisi2_exts pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64" } */
#ifdef _MIPSEB
struct s1 { unsigned long x1 : 5, x2 : 40; };
#else
struct s1 { unsigned long x1 : 19, x2 : 40; };
#endif
void f1 (struct s1 s, unsigned int *x) { *x = (unsigned int) s.x2 / 2; }
/* { dg-final { scan-assembler "\texts\t\[^,\]*,\[^,\]*,19,31\n" } } */
/* { dg-final { scan-assembler-not "\tsll\t" } } */
/* { dg-final { scan-assembler-not "\tdsll\t" } } */
/* { dg-final { scan-assembler-not "\tdsra\t" } } */
