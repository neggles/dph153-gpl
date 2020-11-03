extern void nonpic_nothing (void);
extern int hit_nonpic_addr;
void
nonpic_addr ()
{
  nonpic_receive_fn_addr (&nonpic_nothing);
  hit_nonpic_addr++;
  return;
}
