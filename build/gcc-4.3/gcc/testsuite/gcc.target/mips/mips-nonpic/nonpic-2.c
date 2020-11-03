/* { dg-options "pic-addr.o nonpic-call.o pic-receive-fn-addr.o nonpic-nothing.o pic-nothing.o" } */

extern void nonpic_call ();
extern void pic_nothing ();
extern void abort (void);
extern void exit (int);
int hit_nonpic_call = 0;
int hit_pic_nothing = 0;
int hit_pic_addr = 0;
int hit_nonpic_nothing = 0;
main ()
{
  nonpic_call ();
  pic_nothing ();

  if (hit_nonpic_call != 1)
    abort ();

  if (hit_pic_nothing != 2)
    abort ();

  exit (0);
} 
