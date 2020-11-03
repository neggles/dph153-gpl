extern void pic_nothing (void);
extern void pic_addr (void);
extern int hit_nonpic_call;
void
nonpic_call ()
{
 pic_nothing ();
 pic_addr ();
 hit_nonpic_call++;
}
