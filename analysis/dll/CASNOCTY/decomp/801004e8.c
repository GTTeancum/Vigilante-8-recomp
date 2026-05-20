// addr: 0x801004e8  name: FUN_801004e8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801004e8(uint *param_1,uint param_2,int *param_3)

{
  uint *puVar1;
  undefined4 uVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  
  if (param_2 == 2) {
LAB_8010057c:
    puVar1 = (uint *)FUN_8001d470/*0x8001d470*/(0x9c);
    uVar3 = param_1[5];
    uVar4 = param_1[6];
    uVar5 = param_1[7];
    puVar1[4] = param_1[4];
    puVar1[5] = uVar3;
    puVar1[6] = uVar4;
    puVar1[7] = uVar5;
    uVar3 = param_1[9];
    uVar4 = param_1[10];
    uVar5 = param_1[0xb];
    puVar1[8] = param_1[8];
    puVar1[9] = uVar3;
    puVar1[10] = uVar4;
    puVar1[0xb] = uVar5;
    uVar3 = Terrain_HeightAt/*0x80025400*/(puVar1[9],puVar1[0xb]);
    puVar1[10] = uVar3;
    puVar1[0x19] = (uint)FUN_801002bc;
    puVar1[0x15] = 0x40000;
    *(undefined1 *)(puVar1 + 1) = 7;
    puVar1[0x17] = (uint)&DAT_80100094;
    *(undefined2 *)((int)puVar1 + 0x82) = 4;
    puVar1[0x21] = 0x200;
    *(short *)(puVar1 + 3) = (short)param_2;
    *puVar1 = *puVar1 | 0x184;
    puVar1[0x22] = 0xfffff800;
    puVar1[0x23] = 0;
    FUN_800202f4/*0x800202f4*/(puVar1);
    FUN_80020890/*0x80020890*/(puVar1,0x3c);
    FUN_8001ac08/*0x8001ac08*/(param_1);
    FUN_800207c4/*0x800207c4*/(param_1);
    uVar2 = FUN_8004410c/*0x8004410c*/();
    FUN_8004483c/*0x8004483c*/(uVar2,_DAT_800658fc,0x3b,param_1 + 0x12);
  }
  else {
    if ((param_2 < 3) && (param_2 == 1)) goto LAB_801006a8;
    if ((param_2 == 3) || (param_2 != 5)) {
      if (*(char *)(*param_3 + 4) != '\x02') {
        return 0;
      }
      *param_1 = *param_1 | 0x20;
      FUN_80020890/*0x80020890*/(param_1,0xf);
      goto LAB_8010057c;
    }
  }
  FUN_8001d708/*0x8001d708*/(param_1);
  *param_1 = *param_1 & 0xffffffdf;
  FUN_800207f8/*0x800207f8*/();
LAB_801006a8:
  *param_1 = 0x100;
  *(undefined1 *)(param_1 + 1) = 3;
  return 0;
}

