// addr: 0x800129e8  name: FUN_800129e8

void FUN_800129e8(int param_1,undefined4 param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  iVar1 = FUN_800191e0(uRam00000628);
  iVar2 = 0x140;
  if (param_1 != 0) {
    iVar2 = iRam000006d8;
  }
  if (param_1 == 0) {
    iVar3 = 0x78;
    if (iRam00000010 == 2) {
      iVar3 = 0x98;
    }
  }
  else {
    iVar3 = iRam000006dc / 2;
  }
  FUN_80019cbc(&DAT_8006eef0 + param_1 * 0x84,param_2,(iVar2 - iVar1) / 2,iVar3);
  return;
}

