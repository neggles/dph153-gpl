/* Check the *insvdi_truncated pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64 -fno-tree-sra" } */
#ifdef _MIPSEB
struct s1 { unsigned int x1 : 3, x2 : 20; };
struct s2 { unsigned int x1 : 5, x2 : 10; };
struct s3 { unsigned int x1 : 1; };
struct s4 { unsigned int x1 : 28, x2 : 4; };
#else
struct s1 { unsigned int x1 : 9, x2 : 20; };
struct s2 { unsigned int x1 : 17, x2 : 10; };
struct s3 { unsigned int x2 : 31, x1 : 1; };
struct s4 { unsigned int x2 : 4; };
#endif
void
f1 (struct s1 *p, unsigned long x, unsigned long y)
{ struct s1 s = *p; s.x2 = x / y; *p = s; }
void
f2 (struct s2 *p, unsigned long x, unsigned long y)
{ struct s2 s = *p; s.x2 = x / y; *p = s; }
void
f3 (struct s3 *p, unsigned long x, unsigned long y)
{ struct s3 s = *p; s.x1 = x / y; *p = s; }
void
f4 (struct s4 *p, unsigned long x, unsigned long y)
{ struct s4 s = *p; s.x2 = x / y; *p = s; }
/* { dg-final { scan-assembler "\tdins\t\[^,\]*,\\\$5,9,20\n" } } */
/* { dg-final { scan-assembler "\tdins\t\[^,\]*,\\\$5,17,10\n" } } */
/* { dg-final { scan-assembler "\tdins\t\[^,\]*,\\\$5,31,1\n" } } */
/* { dg-final { scan-assembler "\tdins\t\[^,\]*,\\\$5,0,4\n" } } */
/* { dg-final { scan-assembler-not "\tsll\t" } } */
/* { dg-final { scan-assembler-not "\tdsll\t" } } */
/* { dg-final { scan-assembler-not "\tdsra\t" } } */
/* { dg-final { scan-assembler-not "\tdsrl\t" } } */
/* { dg-final { scan-assembler-not "\tand\t" } } */
