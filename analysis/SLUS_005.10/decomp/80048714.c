// addr: 0x80048714  name: startIntrVSync

code * startIntrVSync(void)

{
  TMR_HRETRACE_MODE = 0x107;
  DAT_8005ffb4 = 0;
  INTR_VB_OBJ_F0(&DAT_8005ff94,8);
  InterruptCallback(0,INTR_VB_OBJ_58);
  return INTR_VB_OBJ_C4;
}

