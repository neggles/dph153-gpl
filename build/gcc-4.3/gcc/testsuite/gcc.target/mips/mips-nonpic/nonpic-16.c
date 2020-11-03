/* { dg-options "nonpic-addr-call.o pic-receive-fn-addr.o pic-addr-call.o nonpic-receive-fn-addr.o pic-nothing.o nonpic-nothing.o" } */

extern void abort (void);
extern void exit (int);
extern void nonpic_addr_call ();
extern void pic_addr_call();
int hit_nonpic_addr_call = 0;
int hit_pic_addr_call = 0;
int hit_nonpic_nothing = 0;
int hit_pic_nothing = 0;
main ()
{
  nonpic_addr_call ();
  pic_addr_call ();

  if (hit_nonpic_addr_call != 1)
    abort ();
  if (hit_pic_addr_call != 1)
    abort ();
  exit (0);
}
