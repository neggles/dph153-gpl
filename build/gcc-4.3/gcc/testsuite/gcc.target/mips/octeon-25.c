/* Check the extv_truncdisi pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64" } */
#ifdef _MIPSEB
struct s1 { long x1 : 5, x2 : 20; };
#else
struct s1 { long x1 : 39, x2 : 20; };
#endif
void f1 (struct s1 s, int *x) { *x = (int) s.x2 >> 2; }
/* { dg-final { scan-assembler "\texts\t\[^,\]*,\[^,\]*,39,19\n" } } */
/* { dg-final { scan-assembler-not "\tsll\t" } } */
/* { dg-final { scan-assembler-not "\tdsll\t" } } */
/* { dg-final { scan-assembler-not "\tdsra\t" } } */
