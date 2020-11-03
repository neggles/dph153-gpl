/* Check the *extendhisi2_hw pattern.  */
/* { dg-mips-options "-march=octeon -O2 -mabi=eabi -mgp32 -mno-abicalls" } */
unsigned int f1 (unsigned int x) { return (short) x; }
/* { dg-final { scan-assembler "\tjr?\t\\\$31\n\tseh\t\\\$2,\\\$4\n" } } */
