/* Check the extzvdi pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=64" } */
#ifdef _MIPSEB
struct s1 { unsigned long x1 : 3, x2 : 31; };
struct s2 { unsigned long x1 : 5, x2 : 50; };
struct s3 { unsigned long x1 : 1, x2 : 1; };
struct s4 { unsigned long x1 : 60, x2 : 3; };
#else
struct s1 { unsigned long x1 : 30, x2 : 31; };
struct s2 { unsigned long x1 : 9, x2 : 50; };
struct s3 { unsigned long x1 : 62, x2 : 1; };
struct s4 { unsigned long x1 : 1, x2 : 3; };
#endif
unsigned long f1 (struct s1 s) { return s.x2; }
unsigned long f2 (struct s2 s) { return s.x2; }
unsigned long f3 (struct s3 s) { return s.x2; }
unsigned long f4 (struct s4 s) { return s.x2; }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tdext\t\\\$2,\\\$4,30,31\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tdext\t\\\$2,\\\$4,9,50\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tdext\t\\\$2,\\\$4,62,1\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tdext\t\\\$2,\\\$4,1,3\n" } } */
