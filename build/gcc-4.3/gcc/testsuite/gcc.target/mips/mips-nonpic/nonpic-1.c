/* { dg-options "pic-nothing.o nonpic-nothing.o" } */

extern void nonpic_nothing ();
extern void pic_nothing ();
int hit_pic_nothing = 0;
int hit_nonpic_nothing = 0;
extern void exit (int);
extern void abort (void);
main ()
{
  nonpic_nothing ();
  pic_nothing ();

  if (hit_nonpic_nothing != 1)
    abort ();

  if (hit_pic_nothing != 1)
    abort ();

  exit (0);
} 
