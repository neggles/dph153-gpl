/* { dg-options "pic-call.o nonpic-addr.o nonpic-receive-fn-addr.o nonpic-nothing.o pic-nothing.o" } */

int hit_pic_call = 0;
int hit_nonpic_addr = 0;
int hit_nonpic_nothing = 0;
int hit_pic_nothing = 0;
extern void nonpic_addr ();
extern void pic_call ();
extern void exit (int);
extern void abort (void);
main ()
{
  pic_call ();
  nonpic_addr ();

  if (hit_pic_call != 1)
    abort ();

  if (hit_nonpic_addr != 2)
    abort ();

  exit (0);
} 
