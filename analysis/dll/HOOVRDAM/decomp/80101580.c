// addr: 0x80101580  name: FUN_80101580

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80101580(uint *param_1,int param_2)

{
  short sVar1;
  uint *puVar2;
  undefined4 uVar3;
  uint uVar4;
  
  if (param_2 != 1) {
    if (param_2 == 0) {
LAB_801015bc:
      if ((((char)param_1[2] != '\0') &&
          (sVar1 = (short)param_1[0x11] + 0x10, *(short *)(param_1 + 0x11) = sVar1, 0x200 < sVar1))
         || (sVar1 = (short)param_1[0x11] + -0x10, *(short *)(param_1 + 0x11) = sVar1,
            sVar1 < -0x200)) {
        puVar2 = (uint *)FUN_8001ac44/*0x8001ac44*/(param_1[0x16],0x25f,0x80,8);
        *(undefined1 *)(puVar2 + 1) = 7;
        *(undefined2 *)(puVar2 + 3) = 0x19;
        *puVar2 = *puVar2 | 0x184;
        puVar2[0x12] = param_1[0x12];
        puVar2[0x13] = param_1[0x13] + 0x15000;
        uVar4 = param_1[0x14];
        puVar2[0x19] = (uint)FUN_80101464;
        puVar2[0x14] = uVar4;
        FUN_80020778/*0x80020778*/(param_1);
        FUN_8002036c/*0x8002036c*/(puVar2);
        FUN_80020890/*0x80020890*/(puVar2,0x200);
        uVar3 = FUN_8004410c/*0x8004410c*/();
        FUN_8004483c/*0x8004483c*/(uVar3,_DAT_800658fc,0x41,puVar2 + 0x12);
      }
      FUN_8001d708/*0x8001d708*/(param_1);
      puVar2 = (uint *)0x1;
    }
    else {
      if (param_2 == 2) goto LAB_8010170c;
      puVar2 = param_1;
      if (param_2 != 3) goto LAB_801015bc;
    }
    *(byte *)(param_1 + 2) = (byte)param_1[2] ^ 1;
    *param_1 = *param_1 | 0x20;
    FUN_80020890/*0x80020890*/(puVar2,0xf0);
    FUN_80020744/*0x80020744*/(param_1);
  }
  *(undefined1 *)(param_1 + 1) = 3;
LAB_8010170c:
  *param_1 = *param_1 & 0xffffffdf;
  return 0;
}

