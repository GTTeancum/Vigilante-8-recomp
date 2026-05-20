// addr: 0x800487d8  name: INTR_VB_OBJ_C4

void INTR_VB_OBJ_C4(int param_1,int param_2)

{
  if (param_2 != (&DAT_8005ff94)[param_1]) {
    (&DAT_8005ff94)[param_1] = param_2;
  }
  return;
}

