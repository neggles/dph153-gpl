extern int hit_pic_addr;
extern void pic_nothing (void);
void
pic_addr ()
{
  pic_receive_fn_addr (&pic_nothing);
  hit_pic_addr++;
  return;
}
