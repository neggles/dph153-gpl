extern void pic_nothing (void);
extern void abort (void);
void
pic_receive_fn_addr (void *x)
{
  if (x != &pic_nothing)
    abort ();
  return;
}
