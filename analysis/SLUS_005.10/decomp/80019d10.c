// addr: 0x80019d10  name: FUN_80019d10

undefined4 FUN_80019d10(short *param_1,int param_2,undefined4 param_3,int param_4)

{
  undefined1 uVar1;
  undefined1 uVar2;
  undefined4 uVar3;
  int iVar4;
  short sVar5;
  undefined1 uVar6;
  int iVar7;
  
  if (*param_1 == 0) {
    uVar3 = 0;
  }
  else {
    iVar4 = (int)*param_1 << 1;
    iVar7 = 0x80;
    if (iVar4 < 0x80) {
      iVar7 = iVar4;
    }
    iVar4 = (int)param_1[1];
    if (iVar4 < 0) {
      iVar4 = iVar4 + 3;
    }
    iVar4 = iVar4 >> 2;
    uVar1 = *(undefined1 *)((int)param_1 + iVar4 + 8);
    uVar2 = *(undefined1 *)((int)param_1 + iVar4 + 9);
    uVar6 = 0;
    if ((param_1[1] & 2U) != 0) {
      uVar6 = 0x5f;
    }
    *(undefined1 *)((int)param_1 + iVar4 + 8) = uVar6;
    *(undefined1 *)((int)param_1 + iVar4 + 9) = 0;
    uVar6 = (undefined1)iVar7;
    *(undefined1 *)(param_2 + 4) = uVar6;
    *(undefined1 *)(param_2 + 5) = uVar6;
    *(undefined1 *)(param_2 + 6) = uVar6;
    FUN_80019c0c(param_2,param_1 + 4,(int)param_1[2],(int)param_1[3],param_3);
    *(undefined1 *)((int)param_1 + iVar4 + 8) = uVar1;
    *(undefined1 *)((int)param_1 + iVar4 + 9) = uVar2;
    sVar5 = 0;
    param_1[1] = param_1[1] + (short)param_4;
    if (0 < *param_1 - param_4) {
      sVar5 = (short)(*param_1 - param_4);
    }
    *param_1 = sVar5;
    uVar3 = 1;
  }
  return uVar3;
}

