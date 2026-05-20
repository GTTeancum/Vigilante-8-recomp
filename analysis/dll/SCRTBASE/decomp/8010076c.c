// addr: 0x8010076c  name: FUN_8010076c

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_8010076c(uint *param_1,uint param_2,int *param_3)

{
  short sVar1;
  undefined2 uVar2;
  uint uVar3;
  undefined4 *puVar4;
  undefined4 uVar5;
  uint uVar6;
  uint uVar7;
  
  if (param_2 != 3) {
    uVar3 = 5;
    if (((3 < param_2) || (uVar3 = 0, param_2 != 0)) && (param_2 == uVar3)) goto LAB_80100944;
    sVar1 = *(short *)((int)param_1 + 0x96);
    *(short *)((int)param_1 + 0x96) = sVar1 + -1;
    if (sVar1 != 1) {
      return 0;
    }
    if ((short)param_1[0x25] == 0) {
      return 0;
    }
    puVar4 = (undefined4 *)FUN_8001ac44/*0x8001ac44*/(param_1[0x16],0x1e1,0x98,8);
    param_3 = (int *)FUN_8001b038/*0x8001b038*/(param_1,0x8000);
    uVar2 = *(undefined2 *)((int)param_1 + 6);
    *(undefined1 *)(puVar4 + 1) = 7;
    *puVar4 = 0x84;
    *(undefined2 *)((int)puVar4 + 6) = uVar2;
    uVar3 = param_1[3];
    puVar4[0x19] = FUN_8010076c;
    *(short *)(puVar4 + 3) = (short)uVar3;
    FUN_8002036c/*0x8002036c*/(puVar4);
    uVar3 = param_1[0x25];
    *(undefined2 *)((int)puVar4 + 0x96) = 4;
    *(short *)(puVar4 + 0x25) = (short)uVar3 + -1;
    uVar3 = param_1[0x23];
    uVar6 = param_1[0x24];
    puVar4[0x22] = param_1[0x22];
    puVar4[0x23] = uVar3;
    puVar4[0x24] = uVar6;
    uVar3 = param_1[5];
    uVar6 = param_1[6];
    uVar7 = param_1[7];
    puVar4[4] = param_1[4];
    puVar4[5] = uVar3;
    puVar4[6] = uVar6;
    puVar4[7] = uVar7;
    uVar3 = param_1[9];
    uVar6 = param_1[10];
    uVar7 = param_1[0xb];
    puVar4[8] = param_1[8];
    puVar4[9] = uVar3;
    puVar4[10] = uVar6;
    puVar4[0xb] = uVar7;
    GTE_RotateLongMatTrans/*0x80043408*/(param_1 + 4,param_3 + 1,puVar4 + 9);
  }
  FUN_80040234/*0x80040234*/(param_1 + 0x12);
  *param_1 = *param_1 | 0x20;
  uVar5 = FUN_8004410c/*0x8004410c*/();
  FUN_8004483c/*0x8004483c*/(uVar5,_DAT_800658fc,0x41,param_1 + 9);
  FUN_8003fea8/*0x8003fea8*/(param_1 + 9,0x8c0c000);
  if (*(char *)(*param_3 + 4) != '\x02') {
    return 0;
  }
  uVar3 = (uint)*(short *)(*param_3 + 6);
  if (-1 < (int)uVar3) {
    return 0;
  }
  MatchSlot_SetCharacter/*0x80012050*/(~uVar3,0x14);
LAB_80100944:
  FUN_800205f8/*0x800205f8*/(param_1);
  return 0;
}

