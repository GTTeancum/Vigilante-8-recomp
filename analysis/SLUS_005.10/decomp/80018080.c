// addr: 0x80018080  name: FUN_80018080

void FUN_80018080(int param_1)

{
  int iVar1;
  
  if (iRam000006c4 != 0) {
    FUN_80017e0c();
  }
  iRam000006c4 = FUN_80017e3c(0,0,0,0,0x400,0x200);
  FUN_80017ec4(iRam000006c4,0x280);
  FUN_80017f4c(*(undefined4 *)(iRam000006c4 + 0x10),0x1e0);
  iVar1 = iRam000006c4;
  *(undefined4 *)(*(int *)(*(int *)(iRam000006c4 + 0x10) + 0x10) + 8) = 1;
  if (param_1 != 0) {
    FUN_80017f4c(*(undefined4 *)(iVar1 + 0x14),0x100);
  }
  return;
}

