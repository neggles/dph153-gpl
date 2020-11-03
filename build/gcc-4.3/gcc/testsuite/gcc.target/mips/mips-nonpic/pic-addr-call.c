extern int hit_pic_addr_call;
extern void nonpic_nothing (void);
extern void nonpic_receive_fn_addr (void *);
void
pic_addr_call (void)
{
  hit_pic_addr_call++;
  nonpic_receive_fn_addr (&nonpic_nothing);
  return;
}
