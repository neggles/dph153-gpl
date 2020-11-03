/* Check the extvsi pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=eabi -mgp32 -mno-abicalls" } */
#ifdef _MIPSEB
struct s1 { int x1 : 3, x2 : 10; };
struct s2 { int x1 : 5, x2 : 14; };
struct s3 { int x1 : 1, x2 : 1; };
struct s4 { int x1 : 28, x2 : 3; };
#else
struct s1 { int x1 : 19, x2 : 10; };
struct s2 { int x1 : 13, x2 : 14; };
struct s3 { int x1 : 30, x2 : 1; };
struct s4 { int x1 : 1, x2 : 3; };
#endif
int f1 (struct s1 s) { return s.x2; }
int f2 (struct s2 s) { return s.x2; }
int f3 (struct s3 s) { return s.x2; }
int f4 (struct s4 s) { return s.x2; }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\texts\t\\\$2,\\\$4,19,9\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\texts\t\\\$2,\\\$4,13,13\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\texts\t\\\$2,\\\$4,30,0\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\texts\t\\\$2,\\\$4,1,2\n" } } */
