// addr: 0x80101284  name: FUN_80101284

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80101284(uint *param_1,undefined4 param_2,uint *param_3)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  
  switch(param_2) {
  case 3:
    uVar4 = *param_3;
    if ((*(char *)(uVar4 + 4) == '\x03') && (param_1[0x20] != 0)) {
      *param_3 = param_1[0x20];
      if (*(code **)(uVar4 + 100) != (code *)0x0) {
        (**(code **)(uVar4 + 100))(uVar4,3);
      }
    }
    FUN_8002239c/*0x8002239c*/(param_1,param_3);
  case 8:
    FUN_80022320/*0x80022320*/(param_1,param_3);
  case 2:
    *param_1 = *param_1 & 0xffffffdf;
  case 6:
  case 1:
    iVar2 = _DAT_800659fc;
    iVar3 = *(int *)(_DAT_800659fc + 0x98);
    *param_1 = *param_1 | 0x108;
    bVar1 = iVar3 != 0;
    *(uint **)(iVar2 + (uint)bVar1 * 4 + 0x98) = param_1;
    *(ushort *)((int)param_1 + 0x42) = (ushort)bVar1 << 0xb;
    return 0;
  default:
    return 0;
  }
}

