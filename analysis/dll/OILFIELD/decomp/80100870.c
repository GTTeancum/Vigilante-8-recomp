// addr: 0x80100870  name: FUN_80100870

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100870(uint *param_1,uint param_2,undefined4 param_3)

{
  uint *puVar1;
  undefined4 uVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  
  if (param_2 != 3) {
    if (((3 < param_2) || (param_2 != 2)) && (param_2 == 8)) goto LAB_801009e0;
    puVar1 = (uint *)FUN_8001d470/*0x8001d470*/(0x9c);
    uVar4 = param_1[5];
    uVar5 = param_1[6];
    uVar6 = param_1[7];
    puVar1[4] = param_1[4];
    puVar1[5] = uVar4;
    puVar1[6] = uVar5;
    puVar1[7] = uVar6;
    uVar4 = param_1[9];
    uVar5 = param_1[10];
    uVar6 = param_1[0xb];
    puVar1[8] = param_1[8];
    puVar1[9] = uVar4;
    puVar1[10] = uVar5;
    puVar1[0xb] = uVar6;
    uVar4 = Terrain_HeightAt/*0x80025400*/(puVar1[9],puVar1[0xb]);
    puVar1[10] = uVar4;
    puVar1[0x19] = (uint)FUN_80100668;
    puVar1[0x15] = 0x40000;
    *(undefined1 *)(puVar1 + 1) = 7;
    *(undefined2 *)(puVar1 + 3) = 10;
    puVar1[0x17] = (uint)&DAT_801000a8;
    puVar1[0x26] = (uint)&DAT_80101148;
    *(undefined2 *)((int)puVar1 + 0x82) = 4;
    puVar1[0x21] = 0x200;
    *puVar1 = *puVar1 | 0x184;
    puVar1[0x22] = 0xfffffa00;
    puVar1[0x23] = 0;
    FUN_800202f4/*0x800202f4*/(puVar1);
    FUN_80020890/*0x80020890*/(puVar1,0x3c);
    FUN_80020890/*0x80020890*/(param_1,900);
    uVar2 = FUN_8004410c/*0x8004410c*/();
    param_3 = 0x3b;
    FUN_8004483c/*0x8004483c*/(uVar2,_DAT_800658fc,0x3b,puVar1 + 9);
  }
  FUN_8002239c/*0x8002239c*/(param_1,param_3);
LAB_801009e0:
  iVar3 = FUN_80022320/*0x80022320*/(param_1,param_3);
  if ((iVar3 != 0) && ((*param_1 & 1) == 0)) {
    FUN_80020890/*0x80020890*/(param_1,0xf0);
    param_1[0x1d] = 0;
  }
  return 0;
}

