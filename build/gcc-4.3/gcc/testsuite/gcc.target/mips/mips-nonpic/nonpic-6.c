/* { dg-options "pic-call.o nonpic-call.o nonpic-addr.o pic-addr.o nonpic-receive-fn-addr.o pic-receive-fn-addr.o nonpic-nothing.o pic-nothing.o" } */

extern void nonpic_call ();
extern void pic_call ();
extern void exit (int);
extern void abort (void);
int hit_pic_call = 0;
int hit_nonpic_call = 0;
int hit_pic_nothing = 0;
int hit_nonpic_nothing = 0;
int hit_nonpic_addr = 0;
int hit_pic_addr = 0;

main ()
{
  pic_call ();
  nonpic_call ();

  if (hit_pic_call != 1)
    abort ();

  if (hit_nonpic_call != 1)
    abort ();

  exit (0);
} 
