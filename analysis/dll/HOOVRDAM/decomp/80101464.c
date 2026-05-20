// addr: 0x80101464  name: FUN_80101464

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80101464(int param_1,undefined4 param_2,int *param_3)

{
  char cVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  
  iVar4 = param_1;
  switch(param_2) {
  case 0:
    *(int *)(param_1 + 0x2c) = *(int *)(param_1 + 0x2c) + -0x3b9a;
    if (param_3 == (int *)0x0) {
      return 0;
    }
    uVar2 = FUN_800449bc/*0x800449bc*/(param_1 + 0x24);
    FUN_80044574/*0x80044574*/((int)*(char *)(param_1 + 5),uVar2);
  case 3:
    iVar3 = *param_3;
    if (*(char *)(iVar3 + 4) == '\x02') {
      iVar4 = iVar3 + 0x24;
      *(undefined4 *)(iVar3 + 0x84) = 0xfff70f80;
      FUN_80040234/*0x80040234*/(iVar4);
      uVar2 = FUN_8004410c/*0x8004410c*/();
      FUN_8004483c/*0x8004483c*/(uVar2,_DAT_800658fc,0x41,iVar4);
switchD_80101494_caseD_2:
      FUN_800205f8/*0x800205f8*/(iVar4);
      param_1 = 1;
      goto switchD_80101494_caseD_1;
    }
    break;
  case 1:
switchD_80101494_caseD_1:
    cVar1 = FUN_8004410c/*0x8004410c*/(param_1);
    *(char *)(iVar4 + 5) = cVar1;
    FUN_800443c8/*0x800443c8*/((int)cVar1,*(undefined4 *)(*(int *)(iVar4 + 0x58) + 8),2,0);
  case 4:
    FUN_800441c8/*0x800441c8*/((int)*(char *)(iVar4 + 5));
    break;
  case 2:
    goto switchD_80101494_caseD_2;
  default:
    break;
  }
  return 0;
}

