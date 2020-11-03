/* Check the *extendqisi2_hw pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=eabi -mgp32 -mno-abicalls" } */
unsigned int f1 (unsigned int x) { return (signed char) x; }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tseb\t\\\$2,\\\$4\n" } } */
