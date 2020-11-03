/* { dg-options "pic-call.o nonpic-addr-call.o nonpic-addr.o nonpic-receive-fn-addr.o pic-receive-fn-addr.o nonpic-nothing.o pic-nothing.o" } */

int hit_nonpic_addr_call = 0;
int hit_pic_call = 0;
int hit_pic_nothing = 0;
int hit_nonpic_nothing = 0;
int hit_nonpic_addr = 0;
extern void exit (int);
extern void abort (void);
extern void nonpic_addr_call ();
extern void pic_call ();
main ()
{
  pic_call ();
  nonpic_addr_call ();

  if (hit_pic_call != 1)
    abort ();

  if (hit_nonpic_addr_call != 1)
    abort ();

  exit (0);
} 
