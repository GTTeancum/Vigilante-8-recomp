// addr: 0x801011b0  name: FUN_801011b0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801011b0(uint *param_1,int param_2,int *param_3)

{
  char cVar1;
  undefined4 uVar2;
  
  if (param_2 != 1) {
    if (param_2 == 0) {
LAB_801011f0:
      if (param_3 == (int *)0x0) {
        return 0;
      }
      param_1[10] = param_1[0x13] +
                    *(short *)(((_DAT_80065310 + (uint)*(byte *)((int)param_1 + 9)) * 0x80 & 0x3f80)
                              + 0x800607b4) * 10;
      if ((*param_1 & 0x20000) == 0) {
        *param_1 = *param_1 & 0xfffeffff;
      }
      *param_1 = *param_1 & 0xfffdffff;
      uVar2 = FUN_800449bc/*0x800449bc*/(param_1 + 9);
      FUN_80044574/*0x80044574*/((int)*(char *)((int)param_1 + 5),uVar2);
    }
    else if (param_2 != 3) {
      if (param_2 == 4) goto LAB_80101338;
      goto LAB_801011f0;
    }
    if ((*param_1 & 0x10000) == 0) {
      FUN_80040234/*0x80040234*/(*param_3 + 0x24);
      uVar2 = FUN_8004410c/*0x8004410c*/();
      FUN_8004483c/*0x8004483c*/(uVar2,_DAT_800658fc,0x41,*param_3 + 0x24);
    }
    *param_1 = *param_1 | 0x30000;
  }
  *(undefined1 *)(param_1 + 1) = 7;
  *(undefined2 *)(param_1 + 3) = 10;
  *param_1 = *param_1 | 0x80;
  param_1[0x13] = param_1[0x13] - 0xa000;
  cVar1 = FUN_8004410c/*0x8004410c*/();
  *(char *)((int)param_1 + 5) = cVar1;
  FUN_800443c8/*0x800443c8*/((int)cVar1,*(undefined4 *)(param_1[0x16] + 8),2,0);
LAB_80101338:
  FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
  return 0;
}

