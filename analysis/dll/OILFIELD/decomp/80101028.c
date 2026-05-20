// addr: 0x80101028  name: FUN_80101028

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80101028(int param_1,int param_2,undefined4 param_3)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int iVar4;
  uint *puVar5;
  undefined2 local_10;
  undefined2 local_e;
  undefined2 local_c;
  undefined2 local_a;
  
  if ((((param_2 == 3) || (param_2 != 8)) && (iVar4 = FUN_8002239c/*0x8002239c*/(param_1,param_3), iVar4 != 0))
     || (iVar4 = FUN_80022320/*0x80022320*/(param_1,param_3), iVar4 != 0)) {
    iVar4 = *(int *)(param_1 + 0x48);
    local_10 = (undefined2)((uint)iVar4 >> 0x10);
    if (iVar4 < 0) {
      local_10 = (undefined2)((uint)(iVar4 + 0xffff) >> 0x10);
    }
    iVar4 = *(int *)(param_1 + 0x50);
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0xffff;
    }
    local_e = (undefined2)((uint)iVar4 >> 0x10);
    local_c = 1;
    local_a = 1;
    FUN_80024718/*0x80024718*/(&local_10,0x8f80);
    piVar3 = (int *)*_DAT_80065a50;
    piVar2 = _DAT_80065a50;
    while (piVar1 = piVar3, piVar1 != (int *)0x0) {
      puVar5 = (uint *)piVar2[2];
      if (*(short *)((int)puVar5 + 6) == *(short *)(param_1 + 6)) {
        *puVar5 = *puVar5 & 0xfffffffd;
      }
      piVar2 = piVar1;
      piVar3 = (int *)*piVar1;
    }
  }
  return 0;
}

