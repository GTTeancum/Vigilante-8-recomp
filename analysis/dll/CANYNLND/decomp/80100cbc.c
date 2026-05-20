// addr: 0x80100cbc  name: FUN_80100cbc

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100cbc(uint *param_1,uint param_2,uint *param_3)

{
  char cVar1;
  uint uVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  uint *puVar7;
  uint *puVar8;
  
  if (param_2 == 2) {
LAB_80100d20:
    cVar1 = (char)param_1[2];
    if (cVar1 == '\x01') {
LAB_80100dd8:
      FUN_800205f8/*0x800205f8*/(param_1[0x1d]);
      FUN_80020890/*0x80020890*/(param_1,0x708);
      *(undefined1 *)(param_1 + 2) = 0;
      *param_1 = *param_1 | 0x20;
    }
    else if (((cVar1 < '\x02') && (cVar1 == '\0')) || (cVar1 != '\x02')) {
      uVar2 = FUN_8001ac44/*0x8001ac44*/(param_1[0x16],0x2bf,0x80,8);
      *(code **)(uVar2 + 100) = FUN_80100be8;
      *(uint *)(uVar2 + 0x48) = param_1[0x12];
      *(uint *)(uVar2 + 0x4c) = param_1[0x13] - 0x14000;
      *(uint *)(uVar2 + 0x50) = param_1[0x14];
      FUN_8002036c/*0x8002036c*/(uVar2);
      param_1[0x1d] = uVar2;
      *param_1 = *param_1 & 0xffffffdf;
      FUN_80020890/*0x80020890*/(param_1,900);
      *(undefined1 *)(param_1 + 2) = 1;
      goto LAB_80100dd8;
    }
    puVar7 = (uint *)param_1[0x1e];
    puVar3 = (undefined4 *)FUN_8001ac44/*0x8001ac44*/(param_1[0x16],*(undefined2 *)((int)param_1 + 10),0x80,8);
    *puVar3 = 0x24;
    uVar2 = param_1[0x1d];
    uVar5 = *(undefined4 *)(uVar2 + 0x4c);
    uVar6 = *(undefined4 *)(uVar2 + 0x50);
    puVar3[0x12] = *(undefined4 *)(uVar2 + 0x48);
    puVar3[0x13] = uVar5;
    puVar3[0x14] = uVar6;
    puVar3[0x19] = 0x8003e80c;
    FUN_8002036c/*0x8002036c*/(puVar3);
    param_3 = puVar3 + 0x12;
    FUN_8003fea8/*0x8003fea8*/(param_3,0x800ff00);
    uVar5 = FUN_8004410c/*0x8004410c*/();
    FUN_800447e8/*0x800447e8*/(uVar5,*(undefined4 *)(param_1[0x16] + 8),1,param_3);
    puVar7[9] = puVar3[0x12];
    puVar7[10] = puVar3[0x13] - 0x64000;
    uVar2 = puVar3[0x14];
    puVar7[0x20] = 0;
    puVar7[0xb] = uVar2;
    puVar7[0x21] = 0;
    puVar7[0x22] = 0;
    *puVar7 = *puVar7 & 0xfcffffdd;
    func_0x80031294();
    param_2 = 0x708;
    DAT_00000009 = 0;
LAB_80100ef4:
    puVar7 = (uint *)*param_3;
    if ((char)puVar7[1] != '\x02') {
      return 0;
    }
    puVar8 = (uint *)puVar7[0x38];
    FUN_800205f8/*0x800205f8*/(param_1[0x1d],param_2);
    *param_1 = *param_1 | 0x20;
    param_1[0x1e] = *param_3;
    uVar5 = FUN_8004410c/*0x8004410c*/();
    FUN_800447e8/*0x800447e8*/(uVar5,*(undefined4 *)(param_1[0x16] + 8),1,param_1 + 0x12);
    FUN_80044574/*0x80044574*/((int)*(char *)((int)puVar7 + 5),0);
    puVar7[0x19] = (uint)FUN_80100c50;
    *puVar7 = *puVar7 & 0xfffffffd | 0x3000000;
    puVar7[0x20] = (int)((param_1[0x12] - puVar7[9]) * 0x80) / 0x78;
    puVar7[0x21] = (int)((param_1[0x13] - puVar7[10]) * 0x80) / 0x78;
    puVar7[0x22] = (int)((param_1[0x14] - puVar7[0xb]) * 0x80) / 0x78;
    puVar7[0x24] = 0;
    puVar7[0x25] = 0x8000;
    puVar7[0x26] = 0;
    if (puVar8 != (uint *)0x0) {
      iVar4 = (short)puVar8[5] * -0x5f5;
      *puVar8 = *puVar8 | 0xc0000;
      if (iVar4 < 0) {
        iVar4 = iVar4 + 0xfff;
      }
      *(short *)(puVar8 + 0x21) = (short)(iVar4 >> 0xc);
      *(undefined2 *)((int)puVar8 + 0x86) = 0xf415;
      iVar4 = (short)puVar8[8] * -0x5f5;
      if (iVar4 < 0) {
        iVar4 = iVar4 + 0xfff;
      }
      *(short *)(puVar8 + 0x22) = (short)(iVar4 >> 0xc);
    }
    *param_1 = *param_1 & 0xfffffffd;
    func_0x80020844(param_1);
    FUN_8001ac08/*0x8001ac08*/(param_1);
    FUN_800207c4/*0x800207c4*/(param_1);
  }
  else {
    if ((param_2 < 3) && (param_2 == 1)) goto LAB_80101230;
    if (param_2 == 3) goto LAB_80100ef4;
    if (param_2 != 5) goto LAB_80100d20;
  }
  puVar7 = (uint *)param_1[0x1e];
  puVar8 = (uint *)puVar7[0x38];
  *param_1 = *param_1 | 2;
  FUN_800207f8/*0x800207f8*/(param_1);
  uVar5 = FUN_8004410c/*0x8004410c*/();
  FUN_800447e8/*0x800447e8*/(uVar5,_DAT_800658fc,0x25,param_1 + 0x12);
  FUN_8003fea8/*0x8003fea8*/(param_1 + 0x12,0x800ff00);
  *puVar7 = *puVar7 | 0x22;
  iVar4 = FUN_80017160/*0x80017160*/();
  uVar2 = FUN_8001ffd4/*0x8001ffd4*/(0x80065a50,(iVar4 * 3 >> 0xf) + -0x19);
  param_1[0x1d] = uVar2;
  puVar7[0x20] = (int)((*(int *)(uVar2 + 0x48) - puVar7[9]) * 0x80) / 300;
  puVar7[0x21] = (int)((*(int *)(param_1[0x1d] + 0x4c) - (puVar7[10] + 0x64000)) * 0x80) / 300;
  puVar7[0x22] = (int)((*(int *)(param_1[0x1d] + 0x50) - puVar7[0xb]) * 0x80) / 300;
  puVar7[0x25] = 0;
  if (puVar8 != (uint *)0x0) {
    *puVar8 = *puVar8 & 0xfffbffff;
  }
  *(undefined1 *)(param_1 + 2) = 2;
LAB_80101230:
  *(undefined1 *)(param_1 + 1) = 3;
  *param_1 = 0x22;
  FUN_80020890/*0x80020890*/(param_1,0x708);
  return 0;
}

