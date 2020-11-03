/* Check the extvdi pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64" } */
#ifdef _MIPSEB
struct s1 { long x1 : 3, x2 : 31; };
struct s2 { long x1 : 5, x2 : 32; };
struct s3 { long x1 : 1, x2 : 1; };
struct s4 { long x1 : 60, x2 : 3; };
#else
struct s1 { long x1 : 30, x2 : 31; };
struct s2 { long x1 : 27, x2 : 32; };
struct s3 { long x1 : 62, x2 : 1; };
struct s4 { long x1 : 1, x2 : 3; };
#endif
long f1 (struct s1 s) { return s.x2; }
long f2 (struct s2 s) { return s.x2; }
long f3 (struct s3 s) { return s.x2; }
long f4 (struct s4 s) { return s.x2; }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\texts\t\\\$2,\\\$4,30,30\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\texts\t\\\$2,\\\$4,27,31\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\texts\t\\\$2,\\\$4,62,0\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\texts\t\\\$2,\\\$4,1,2\n" } } */
