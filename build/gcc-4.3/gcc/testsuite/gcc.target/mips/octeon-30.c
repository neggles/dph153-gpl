/* Check the *insvdi_zero_extended pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64 -fno-tree-sra" } */
#ifdef _MIPSEB
struct s1 { unsigned long x1 : 3, x2 : 31; };
struct s2 { unsigned long x1 : 5, x2 : 20; };
struct s3 { unsigned long x1 : 1; };
struct s4 { unsigned long x1 : 60, x2 : 4; };
#else
struct s1 { unsigned long x1 : 30, x2 : 31; };
struct s2 { unsigned long x1 : 39, x2 : 20; };
struct s3 { unsigned long x2 : 63, x1 : 1; };
struct s4 { unsigned long x2 : 4; };
#endif
struct s1 f1 (struct s1 s, unsigned int x) { s.x2 = x; return s; }
struct s2 f2 (struct s2 s, unsigned int x) { s.x2 = x; return s; }
struct s3 f3 (struct s3 s, unsigned int x) { s.x1 = x; return s; }
struct s4 f4 (struct s4 s, unsigned int x) { s.x2 = x; return s; }
/* { dg-final { scan-assembler "\tdins\t\[^,\]*,\\\$5,30,31\n" } } */
/* { dg-final { scan-assembler "\tdins\t\[^,\]*,\\\$5,39,20\n" } } */
/* { dg-final { scan-assembler "\tdins\t\[^,\]*,\\\$5,63,1\n" } } */
/* { dg-final { scan-assembler "\tdins\t\[^,\]*,\\\$5,0,4\n" } } */
/* { dg-final { scan-assembler-not "\tsll\t" } } */
/* { dg-final { scan-assembler-not "\tdsll\t" } } */
/* { dg-final { scan-assembler-not "\tdsra\t" } } */
/* { dg-final { scan-assembler-not "\tdsrl\t" } } */
/* { dg-final { scan-assembler-not "\tand\t" } } */
