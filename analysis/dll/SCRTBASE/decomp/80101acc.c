// addr: 0x80101acc  name: FUN_80101acc

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80101acc(int param_1,uint param_2,int *param_3)

{
  uint *puVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  
  if (param_2 != 2) {
    if ((param_2 < 3) && (puVar1 = (uint *)0x3, param_2 == 1)) goto LAB_80101c2c;
    if ((param_2 == 3) || (iVar2 = param_1, param_2 != 8)) {
      puVar1 = (uint *)param_3[3];
      iVar2 = param_1;
      if (((byte)puVar1[1] == param_2) && (iVar4 = *param_3, *(char *)(iVar4 + 4) == '\x02')) {
        uVar3 = 0x74;
        if (*(int *)(iVar4 + 0x80) < 0) {
          uVar3 = 0x75;
        }
        iVar2 = func_0x80021808(uVar3);
        FUN_8001f9cc/*0x8001f9cc*/(iVar2,**(undefined2 **)(iVar2 + 0x60));
        *(undefined1 *)(iVar2 + 8) = 1;
        FUN_80020890/*0x80020890*/(iVar2,600);
        FUN_80020890/*0x80020890*/(param_1,600);
        *puVar1 = *puVar1 | 0x20;
        uVar3 = FUN_8004410c/*0x8004410c*/();
        param_3 = (int *)0x14;
        FUN_800447e8/*0x800447e8*/(uVar3,_DAT_800658fc,0x14,iVar4 + 0x24);
        iVar2 = 1;
      }
      FUN_8002239c/*0x8002239c*/(iVar2,param_3);
      iVar2 = 1;
    }
    iVar2 = FUN_80022320/*0x80022320*/(iVar2,param_3);
    if (iVar2 == 0) {
      return 0;
    }
    func_0x80020844(param_1);
  }
  puVar1 = *(uint **)(param_1 + 0x38);
  *puVar1 = *puVar1 & 0xffffffdf;
LAB_80101c2c:
  *(char *)(*(int *)(param_1 + 0x38) + 4) = (char)puVar1;
  return 0;
}

