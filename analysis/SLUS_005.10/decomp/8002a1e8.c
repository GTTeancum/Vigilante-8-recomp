// addr: 0x8002a1e8  name: FUN_8002a1e8

void FUN_8002a1e8(uint param_1,uint *param_2,uint param_3,uint param_4)

{
  uint uVar1;
  int iVar2;
  int *piVar3;
  uint *in_t1;
  
  piVar3 = &DAT_800a1e20;
  if (iRam00000004 != 0) {
    piVar3 = &DAT_800a2324;
  }
  iVar2 = *piVar3;
  if (iVar2 < 0x40) {
    in_t1 = (uint *)(piVar3 + iVar2 * 5 + 1);
    *piVar3 = iVar2 + 1;
  }
  in_t1[2] = param_1;
  in_t1[3] = param_4;
  in_t1[4] = param_3;
  uVar1 = *param_2;
  *param_2 = (uint)in_t1 & 0xffffff;
  *in_t1 = (uint)*(byte *)((int)in_t1 + 3) << 0x18 | uVar1;
  return;
}

