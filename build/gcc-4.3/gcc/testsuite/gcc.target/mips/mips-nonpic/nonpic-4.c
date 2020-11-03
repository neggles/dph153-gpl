/* { dg-options "pic-addr.o pic-receive-fn-addr.o nonpic-addr-call.o nonpic-nothing.o pic-nothing.o" } */

int hit_nonpic_addr_call = 0;
int hit_pic_nothing = 0;
int hit_nonpic_nothing = 0;
int hit_pic_addr = 0;
int hit_nonpic_addr = 0;
extern void nonpic_addr_call ();
extern void pic_nothing ();
extern void exit (int);
extern void abort (void);
main ()
{
  nonpic_addr_call ();
  pic_nothing ();

  if (hit_nonpic_addr_call != 1)
    abort ();

  if (hit_pic_nothing != 1)
    abort ();

  exit (0);
} 
