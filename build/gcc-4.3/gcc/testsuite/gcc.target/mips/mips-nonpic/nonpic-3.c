/* { dg-options "nonpic-addr.o nonpic-receive-fn-addr.o nonpic-nothing.o pic-nothing.o" } */

extern void nonpic_addr ();
extern void pic_nothing ();
extern void abort (void);
extern void exit (int);
int hit_nonpic_addr = 0;
int hit_pic_nothing = 0;
int hit_nonpic_nothing = 0;
main ()
{
  nonpic_addr ();
  pic_nothing ();

  if (hit_nonpic_addr != 1)
    abort ();

  if (hit_pic_nothing != 1)
    abort ();

  exit (0);
} 
