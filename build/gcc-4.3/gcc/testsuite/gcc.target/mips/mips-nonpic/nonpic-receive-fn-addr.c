extern void nonpic_nothing (void);
extern void abort (void);
void
nonpic_receive_fn_addr (void *x)
{
  if (x != &nonpic_nothing)
    abort ();
}
