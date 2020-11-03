/* Check the *extz_truncdisi2 pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64" } */
#ifdef _MIPSEB
struct s1 { unsigned long x1 : 5, x2 : 20; };
#else
struct s1 { unsigned long x1 : 39, x2 : 20; };
#endif
void f1 (struct s1 s, unsigned int *x) { *x = (unsigned int) s.x2 / 2; }
/* { dg-final { scan-assembler "\tdext\t\[^,\]*,\[^,\]*,39,20\n" } } */
/* { dg-final { scan-assembler-not "\tsll\t" } } */
/* { dg-final { scan-assembler-not "\tdsll\t" } } */
/* { dg-final { scan-assembler-not "\tdsra\t" } } */
