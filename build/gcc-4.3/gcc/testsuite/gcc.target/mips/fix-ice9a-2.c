/* Check that nops are inserted before and after the fp operations.  */
/* { dg-do compile } */
/* { dg-options "-mfix-ice9a -O3" } */
/* { dg-mips-options "-mhard-float" } */

typedef float TYPE;
#include "fix-ice9a.h"

/* Test the madd insn.  */
/* { dg-final { scan-assembler-not "\tmovn.s\t\\\$f30, \\\$f28, \\\$0\n\tmovn.s\t\\\$f30, \\\$f28, \\\$0\n\tmovn.s\t\\\$f30, \\\$f28, \\\$0\n\tmovn.s\t\\\$f30, \\\$f28, \\\$0\n\tmovn.s\t\\\$f30, \\\$f28, \\\$0\n\tmadd.s\t\\\$f(\[0-9]{1,2}),\\\$f(\[0-9]{1,2}),\\\$f(\[0-9]{1,2}),\\\$f(\[0-9]{1,2})\n\tmovn.s\t\\\$f30, \\\$f(\[0-9]{1,2}), \\\$0\n" } } */

/* Test the sqrt insn.  */
/* { dg-final { scan-assembler-not "\tsqrt.s\t\\\$f(\[0-9]{1,2}),\\\$f(\[0-9]{1,2})\n\tmovn.s\t\\\$f30, \\\$f(\[0-9]{1,2}), \\\$0\n" } } */

/* Test the mul insn.  */
/* { dg-final { scan-assembler-not "\tmovn.s\t\\\$f30, \\\$f28, \\\$0\n\tmovn.s\t\\\$f30, \\\$f28, \\\$0\n\tmovn.s\t\\\$f30, \\\$f28, \\\$0\n\tmovn.s\t\\\$f30, \\\$f28, \\\$0\n\tmovn.s\t\\\$f30, \\\$f28, \\\$0\n\tmul.s\t\\\$f(\[0-9]{1,2}),\\\$f(\[0-9]{1,2}),\\\$f(\[0-9]{1,2})\n\tmovn.s\t\\\$f30, \\\$f(\[0-9]{1,2}), \\\$0\n" } } */

