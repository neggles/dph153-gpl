/* Check the extv_truncdihi pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64" } */
#ifdef _MIPSEB
struct s1 { long x1 : 5, x2 : 31; };
#else
struct s1 { long x1 : 28, x2 : 31; };
#endif
void f1 (struct s1 s, int *x) { *x = (short) s.x2 + 1; }
/* { dg-final { scan-assembler "\texts\t\[^,\]*,\[^,\]*,28,30\n" } } */
/* { dg-final { scan-assembler-not "\tsll\t" } } */
/* { dg-final { scan-assembler-not "\tdsll\t" } } */
/* { dg-final { scan-assembler-not "\tdsra\t" } } */
