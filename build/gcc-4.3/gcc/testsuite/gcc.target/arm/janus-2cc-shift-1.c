/* Check that a nop is inserted after a shift taking a register operand.  */
/* { dg-do compile } */
/* { dg-options "-mfix-janus-2cc" } */
/* { dg-require-effective-target arm_not_thumb } */
int foo(int x)
{
  int y;
  int z;
  
  y = x << 4;
  z = y << x;

  return y+z;
}
/* { dg-final { scan-assembler "\tmov\tr\[0-9], r\[0-9], asl r\[0-9]\n\tnop\n" } } */
/* { dg-final { scan-assembler-not "\tmov\tr\[0-9], r\[0-9], asl #4\n\tnop\n" } } */

