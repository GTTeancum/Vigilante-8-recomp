// addr: 0x8010129c  name: FUN_8010129c

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_8010129c(uint *param_1,undefined4 param_2,int *param_3)

{
  undefined2 uVar1;
  char cVar2;
  ushort uVar3;
  uint uVar4;
  int iVar5;
  undefined4 *puVar6;
  int iVar7;
  uint uVar8;
  undefined4 uVar9;
  int iVar10;
  uint uVar11;
  
  switch(param_2) {
  case 0:
    if ((short)param_1[0x21] < 0) {
      iVar10 = FUN_80017160/*0x80017160*/(param_1);
      iVar10 = FUN_8001ffd4/*0x8001ffd4*/(0x80065a50,(iVar10 * 3 >> 0xf) + -0x19);
      FUN_80042ef0/*0x80042ef0*/(param_1 + 0x21,param_1 + 0x12,iVar10 + 0x48,0xffffffff,0);
    }
    uVar4 = func_0x800430a0(param_1,param_1 + 0x21,0x10000);
    iVar5 = (uVar4 & 0xfff) * 4;
    iVar10 = *(short *)(iVar5 + -0x7ff9f84c) * 0x5f5;
    if (iVar10 < 0) {
      iVar10 = iVar10 + 0xfff;
    }
    uVar4 = param_1[0x12] + (iVar10 >> 0xc);
    param_1[0x12] = uVar4;
    iVar10 = *(short *)(iVar5 + -0x7ff9f84a) * 0x5f5;
    if (iVar10 < 0) {
      iVar10 = iVar10 + 0xfff;
    }
    param_1[0x14] = param_1[0x14] + (iVar10 >> 0xc);
    uVar4 = Terrain_HeightAt/*0x80025400*/(uVar4);
    uVar8 = param_1[0xe];
    param_1[0x13] = uVar4;
    *(short *)((int)param_1 + 0x42) = *(short *)((int)param_1 + 0x42) + 0x44;
    *(short *)(uVar8 + 0x42) = *(short *)(uVar8 + 0x42) + 0x44;
    if ((_DAT_80065310 - (uint)*(byte *)((int)param_1 + 9) & 0xf) == 0) {
      puVar6 = (undefined4 *)FUN_8001ac44/*0x8001ac44*/(param_1[0x16],0x2ba,0x80,8);
      *(undefined1 *)(puVar6 + 1) = 1;
      *puVar6 = 0x34;
      iVar10 = FUN_80017160/*0x80017160*/();
      puVar6[0x12] = param_1[0x12] + (iVar10 * 0x5000 >> 0xf) + -0x2800;
      puVar6[0x13] = param_1[0x13];
      iVar10 = FUN_80017160/*0x80017160*/();
      puVar6[0x14] = param_1[0x14] + (iVar10 * 0x5000 >> 0xf) + -0x2800;
      puVar6[0x19] = 0x8003e80c;
      FUN_8002036c/*0x8002036c*/();
      uVar8 = 1;
    }
    uVar4 = FUN_80017160/*0x80017160*/(uVar8);
    if ((uVar4 & 0x1f) == 0) {
      uVar4 = FUN_80017160/*0x80017160*/();
      uVar9 = 0x2b9;
      if ((uVar4 & 1) != 0) {
        uVar9 = 0x2b8;
      }
      puVar6 = (undefined4 *)FUN_8001ac44/*0x8001ac44*/(param_1[0x16],uVar9,0x94,0);
      iVar5 = FUN_80017160/*0x80017160*/();
      uVar4 = FUN_80017160/*0x80017160*/();
      iVar7 = (uVar4 & 0xfff) * 4;
      iVar10 = *(short *)(iVar7 + -0x7ff9f84c) * 0x11e1;
      if (iVar10 < 0) {
        iVar10 = iVar10 + 0xfff;
      }
      puVar6[0x22] = iVar10 >> 0xc;
      iVar10 = *(short *)((((iVar5 << 8) >> 0xf) + 0x100U & 0xfff) * 4 + -0x7ff9f84c) * -0x11e1;
      if (iVar10 < 0) {
        iVar10 = iVar10 + 0xfff;
      }
      puVar6[0x23] = iVar10 >> 0xc;
      iVar10 = *(short *)(iVar7 + -0x7ff9f84a) * 0x11e1;
      if (iVar10 < 0) {
        iVar10 = iVar10 + 0xfff;
      }
      puVar6[0x24] = iVar10 >> 0xc;
      uVar3 = FUN_80017160/*0x80017160*/();
      *(ushort *)(puVar6 + 0x20) = uVar3 & 0xff;
      uVar3 = FUN_80017160/*0x80017160*/();
      *(ushort *)((int)puVar6 + 0x82) = uVar3 & 0xff;
      uVar3 = FUN_80017160/*0x80017160*/();
      *(ushort *)(puVar6 + 0x21) = uVar3 & 0xff;
      uVar1 = *(undefined2 *)((int)param_1 + 6);
      *(undefined1 *)(puVar6 + 1) = 1;
      *puVar6 = 0x80;
      puVar6[0x19] = 0x8003eab0;
      *(undefined2 *)((int)puVar6 + 6) = uVar1;
      uVar4 = param_1[5];
      uVar8 = param_1[6];
      uVar11 = param_1[7];
      puVar6[4] = param_1[4];
      puVar6[5] = uVar4;
      puVar6[6] = uVar8;
      puVar6[7] = uVar11;
      uVar4 = param_1[9];
      uVar8 = param_1[10];
      uVar11 = param_1[0xb];
      puVar6[8] = param_1[8];
      puVar6[9] = uVar4;
      puVar6[10] = uVar8;
      puVar6[0xb] = uVar11;
      *(undefined1 *)((int)puVar6 + 0x87) = 2;
      FUN_8001dc1c/*0x8001dc1c*/(puVar6);
      FUN_800202f4/*0x800202f4*/(puVar6);
    }
    if (param_3 == (int *)0x0) {
      return 0;
    }
    iVar10 = FUN_800449bc/*0x800449bc*/(param_1 + 0x12);
    if (iVar10 != 0) {
      iVar5 = (int)*(char *)((int)param_1 + 5);
      if (iVar5 != 0) {
        FUN_80044574/*0x80044574*/(iVar5,iVar10);
      }
      cVar2 = FUN_8004410c/*0x8004410c*/(iVar5 != 0);
      *(char *)((int)param_1 + 5) = cVar2;
      FUN_800443c8/*0x800443c8*/((int)cVar2,*(undefined4 *)(param_1[0x16] + 8),4,iVar10);
    }
    if (*(char *)((int)param_1 + 5) != '\0') {
      FUN_800441c8/*0x800441c8*/();
      *(undefined1 *)((int)param_1 + 5) = 0;
    }
    FUN_8001d708/*0x8001d708*/(param_1);
    FUN_8001d708/*0x8001d708*/(param_1[0xe]);
  case 3:
    iVar10 = *param_3;
    if (*(char *)(iVar10 + 4) == '\x02') {
      *(undefined4 *)(iVar10 + 0x84) = 0xfffd0580;
      *(int *)(iVar10 + 0x94) = *(int *)(iVar10 + 0x94) + 4000;
switchD_801012e0_caseD_6:
      goto switchD_801012e0_caseD_1;
    }
    break;
  case 1:
switchD_801012e0_caseD_1:
    *(undefined1 *)(param_1 + 1) = 3;
    *(undefined2 *)(param_1 + 0x21) = 0xffff;
    *param_1 = *param_1 | 0x80;
  case 4:
    FUN_80042f5c/*0x80042f5c*/(param_1 + 0x21);
  default:
    break;
  case 6:
    goto switchD_801012e0_caseD_6;
  }
  return 0;
}

