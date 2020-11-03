/* Check the insvsi pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=eabi -mno-abicalls -mgp32 -fno-tree-sra" } */
#ifdef _MIPSEB
struct s1 { unsigned int x1 : 3, x2 : 20; };
struct s2 { unsigned int x1 : 5, x2 : 10; };
struct s3 { unsigned int x1 : 1, pad : 1; };
struct s4 { unsigned int x1 : 28, x2 : 4; };
#else
struct s1 { unsigned int x1 : 9, x2 : 20; };
struct s2 { unsigned int x1 : 17, x2 : 10; };
struct s3 { unsigned int x2 : 31, x1 : 1; };
struct s4 { unsigned int x2 : 4, pad : 1; };
#endif
struct s1 f1 (struct s1 s, unsigned int x) { s.x2 = x; return s; }
struct s2 f2 (struct s2 s, unsigned int x) { s.x2 = x; return s; }
struct s3 f3 (struct s3 s, unsigned int x) { s.x1 = x; return s; }
struct s4 f4 (struct s4 s, unsigned int x) { s.x2 = x; return s; }
/* { dg-final { scan-assembler "\tins\t\[^,\]*,\\\$5,9,20\n" } } */
/* { dg-final { scan-assembler "\tins\t\[^,\]*,\\\$5,17,10\n" } } */
/* { dg-final { scan-assembler "\tins\t\[^,\]*,\\\$5,31,1\n" } } */
/* { dg-final { scan-assembler "\tins\t\[^,\]*,\\\$5,0,4\n" } } */
