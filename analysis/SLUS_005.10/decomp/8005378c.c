// addr: 0x8005378c  name: SPRINTF_OBJ_788

void SPRINTF_OBJ_788(undefined4 param_1,int param_2)

{
  int unaff_s2;
  int unaff_s3;
  
  if (param_2 == 0x25) {
    *(undefined1 *)(unaff_s3 + unaff_s2) = 0x25;
    SPRINTF_OBJ_82C();
    return;
  }
  *(undefined1 *)(unaff_s3 + unaff_s2) = 0;
  return;
}

