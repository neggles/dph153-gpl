extern int hit_pic_call;
extern void nonpic_nothing (void);
extern void nonpic_addr (void);
void
pic_call ()
{
 nonpic_nothing ();
 nonpic_addr ();
 hit_pic_call++;
}
