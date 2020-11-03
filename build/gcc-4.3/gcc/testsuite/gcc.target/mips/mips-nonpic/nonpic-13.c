/* { dg-options "pic-addr-call.o nonpic-receive-fn-addr.o nonpic-nothing.o" } */

int hit_pic_addr_call = 0;
int hit_nonpic_nothing = 0;
extern void nonpic_nothing ();
extern void pic_addr_call();
extern void exit (int);
extern void abort (void);
main ()
{
  nonpic_nothing ();
  pic_addr_call ();

  if (hit_nonpic_nothing != 1)
    abort ();

  if (hit_pic_addr_call != 1)
    abort ();

  exit (0);
} 
