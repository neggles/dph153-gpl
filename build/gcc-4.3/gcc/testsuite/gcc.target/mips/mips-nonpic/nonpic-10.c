/* { dg-options "nonpic-call.o pic-addr.o pic-receive-fn-addr.o nonpic-nothing.o pic-nothing.o" } */

extern void nonpic_call ();
extern void pic_addr();
int hit_nonpic_call = 0;
int hit_pic_addr = 0;
int hit_pic_nothing = 0;
int hit_nonpic_nothing = 0;
extern void exit (int);
extern void abort (void);

main ()
{
  nonpic_call ();
  pic_addr ();

  if (hit_nonpic_call != 1)
    abort ();

  if (hit_pic_addr != 2)
    abort ();

  exit (0);

} 
