/* Check the extzvsi pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=eabi -mgp32 -mno-abicalls" } */
#ifdef _MIPSEB
struct s1 { unsigned int x1 : 3, x2 : 10; };
struct s2 { unsigned int x1 : 5, x2 : 14; };
struct s3 { unsigned int x1 : 1, x2 : 1; };
struct s4 { unsigned int x1 : 28, x2 : 3; };
#else
struct s1 { unsigned int x1 : 19, x2 : 10; };
struct s2 { unsigned int x1 : 13, x2 : 14; };
struct s3 { unsigned int x1 : 30, x2 : 1; };
struct s4 { unsigned int x1 : 1, x2 : 3; };
#endif
unsigned int f1 (struct s1 s) { return s.x2; }
unsigned int f2 (struct s2 s) { return s.x2; }
unsigned int f3 (struct s3 s) { return s.x2; }
unsigned int f4 (struct s4 s) { return s.x2; }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\text\t\\\$2,\\\$4,19,10\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\text\t\\\$2,\\\$4,13,14\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\text\t\\\$2,\\\$4,30,1\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\text\t\\\$2,\\\$4,1,3\n" } } */
