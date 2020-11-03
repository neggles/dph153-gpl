/* Check the mov_u[ls]w patterns.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=eabi -mno-abi-calls -mgp32 -mocteon-useun -meb" } */
struct __attribute__((packed)) s { unsigned int x; };
unsigned int f1 (struct s *s) { return s[0].x; };
void f2 (struct s *s, unsigned long x) { s[10].x = x; }
void f3 (struct s *s) { s[100].x = 0; }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tulw\t\\\$2,0\\(\\\$4\\)\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tusw\t\\\$5,40\\(\\\$4\\)\n" } } */
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tusw\t\\\$0,400\\(\\\$4\\)\n" } } */
