/* { dg-options "pic-addr.o pic-receive-fn-addr.o nonpic-nothing.o pic-nothing.o" } */

extern void nonpic_nothing ();
extern void pic_addr();
extern void exit (int);
extern void abort (void);
int hit_pic_addr = 0;
int hit_nonpic_nothing = 0;
int hit_pic_nothing = 0;
int hit_pic_receive_fn_addr = 0;
main ()
{
  pic_addr ();
  nonpic_nothing ();
  exit (0);
} 
