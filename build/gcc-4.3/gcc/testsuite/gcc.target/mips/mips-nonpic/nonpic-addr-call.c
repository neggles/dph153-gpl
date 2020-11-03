extern int hit_nonpic_addr_call;
extern void pic_nothing (void);
extern void pic_receive_fn_addr (void *);
void
nonpic_addr_call (void)
{
  hit_nonpic_addr_call++;
  pic_receive_fn_addr (&pic_nothing);
  return;
}
