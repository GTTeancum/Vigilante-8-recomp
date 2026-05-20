// addr: 0x8003733c  name: FUN_8003733c

undefined4 FUN_8003733c(uint *param_1,undefined4 param_2,int *param_3)

{
  short sVar1;
  char cVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 *puVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  uint *puVar9;
  undefined1 auStack_28 [16];
  
  switch(param_2) {
  case 0:
    param_1[0x12] = param_1[0x12] + param_1[0x22];
    param_1[0x13] = param_1[0x13] + param_1[0x23];
    param_1[0x14] = param_1[0x14] + param_1[0x24];
    iVar7 = FUN_80025400(param_1[0x12],param_1[0x14]);
    if ((int)(iVar7 - param_1[0x15]) < (int)param_1[0x13]) {
      param_1[0x13] = iVar7 - param_1[0x15];
    }
    *(short *)((int)param_1 + 0x42) = *(short *)((int)param_1 + 0x42) + 0x88;
    *(short *)(param_1 + 0x10) = (short)param_1[0x10] + 0x5b;
    if (param_3 != (int *)0x0) {
      FUN_8001d708(param_1);
      uVar4 = FUN_800446dc(param_1 + 9);
      FUN_80044574((int)*(char *)((int)param_1 + 5),uVar4);
    }
    iVar7 = *(int *)(param_1[0x21] + 0x48) - param_1[0x12];
    if (iVar7 < 0) {
      iVar7 = iVar7 + 0xff;
    }
    uVar6 = param_1[0x22] + (iVar7 >> 8);
    uVar8 = 0xffffec55;
    if ((-0x13ac < (int)uVar6) && (uVar8 = 0x13ab, (int)uVar6 < 0x13ac)) {
      uVar8 = uVar6;
    }
    param_1[0x22] = uVar8;
    iVar7 = *(int *)(param_1[0x21] + 0x4c) - param_1[0x13];
    if (iVar7 < 0) {
      iVar7 = iVar7 + 0xff;
    }
    uVar6 = param_1[0x23] + (iVar7 >> 8);
    uVar8 = 0xffffec55;
    if ((-0x13ac < (int)uVar6) && (uVar8 = 0x13ab, (int)uVar6 < 0x13ac)) {
      uVar8 = uVar6;
    }
    param_1[0x23] = uVar8;
    iVar7 = *(int *)(param_1[0x21] + 0x50) - param_1[0x14];
    if (iVar7 < 0) {
      iVar7 = iVar7 + 0xff;
    }
    uVar6 = param_1[0x24] + (iVar7 >> 8);
    uVar8 = 0xffffec55;
    if ((-0x13ac < (int)uVar6) && (uVar8 = 0x13ab, (int)uVar6 < 0x13ac)) {
      uVar8 = uVar6;
    }
    sVar1 = *(short *)((int)param_1 + 0x96);
    param_1[0x24] = uVar8;
    *(short *)((int)param_1 + 0x96) = sVar1 + -1;
    if (sVar1 != 1) {
      return 0;
    }
    puVar9 = (uint *)param_1[0xe];
    if (puVar9 == (uint *)0x0) {
      FUN_800441c8((int)*(char *)((int)param_1 + 5));
      FUN_800205f8(param_1);
      return 0xffffffff;
    }
    FUN_8001d564(puVar9);
    puVar9[0x12] = puVar9[0x12] + param_1[0x12];
    puVar9[0x13] = puVar9[0x13] + param_1[0x13];
    puVar9[0x14] = puVar9[0x14] + param_1[0x14];
    puVar9[0x19] = (uint)&LAB_800372b0;
    *puVar9 = *puVar9 | 0x84;
    iVar7 = FUN_80017160();
    puVar9[0x22] = ((iVar7 << 10) >> 0xf) - 0x200;
    iVar7 = FUN_80017160();
    puVar9[0x23] = -(((iVar7 << 10) >> 0xf) + 0x400);
    iVar7 = FUN_80017160();
    puVar9[0x24] = ((iVar7 << 10) >> 0xf) - 0x200;
    *(undefined1 *)((int)puVar9 + 0x86) = 0xf0;
    FUN_8002036c();
    *(undefined2 *)((int)param_1 + 0x96) = 0xf;
    break;
  case 1:
    iVar7 = 0;
    do {
      puVar5 = (undefined4 *)FUN_8001ac44(param_1[0x16],0x3a,0x94,8);
      iVar3 = FUN_80017160();
      puVar5[0x12] = ((iVar3 << 0x10) >> 0xf) + -0x8000;
      iVar3 = FUN_80017160();
      puVar5[0x13] = ((iVar3 << 0x10) >> 0xf) + -0x8000;
      iVar3 = FUN_80017160();
      iVar7 = iVar7 + 1;
      puVar5[0x14] = ((iVar3 << 0x10) >> 0xf) + -0x8000;
      puVar5[9] = puVar5[0x12];
      puVar5[10] = puVar5[0x13];
      puVar5[0xb] = puVar5[0x14];
      *puVar5 = 0x410;
      iVar3 = FUN_80017160();
      *(char *)((int)puVar5 + 0x86) = (char)((iVar3 << 3) >> 0xf) + '\x18';
      FUN_8001d4f0(param_1);
    } while (iVar7 < 0x10);
    param_1[0x17] = (uint)&DAT_80010588;
    param_1[0x15] = 0x8000;
    cVar2 = FUN_8004410c();
    *(char *)((int)param_1 + 5) = cVar2;
    FUN_800447e8((int)cVar2,*(undefined4 *)(param_1[0x16] + 8),0,param_1 + 0x12);
    iRam00000758 = iRam00000758 + 1;
    break;
  case 3:
    FUN_8001f5a0(param_1,param_3);
    FUN_80043408(param_1 + 4,param_3 + 5,auStack_28);
    FUN_8003fd24(auStack_28,0x17);
    param_1[0x22] =
         param_1[0x22] +
         (((int)((uint)*(ushort *)(param_3 + 8) << 0x10) >> 0x10) -
          ((int)((uint)*(ushort *)(param_3 + 8) << 0x10) >> 0x1f) >> 1);
    iVar7 = (uint)*(ushort *)((int)param_3 + 0x22) << 0x10;
    param_1[0x23] = param_1[0x23] + ((iVar7 >> 0x10) - (iVar7 >> 0x1f) >> 1);
    param_1[0x24] =
         param_1[0x24] +
         (((int)((uint)*(ushort *)(param_3 + 9) << 0x10) >> 0x10) -
          ((int)((uint)*(ushort *)(param_3 + 9) << 0x10) >> 0x1f) >> 1);
    iVar7 = *param_3;
    if (*(char *)(iVar7 + 4) != '\x02') {
      return 0;
    }
    *(int *)(iVar7 + 0x80) = *(int *)(iVar7 + 0x80) >> 2;
    iVar3 = FUN_8001d748(iVar7,iVar7 + 0x48,0,0);
    if (iVar3 + -0xf000 < *(int *)(iVar7 + 0x28)) {
      *(undefined4 *)(iVar7 + 0x84) = 0xfffa0a80;
    }
    *(int *)(iVar7 + 0x88) = *(int *)(iVar7 + 0x88) >> 2;
    uVar4 = FUN_8004410c();
    FUN_800447e8(uVar4,uRam000005f8,0x3e,param_1 + 0x12);
    if (*(short *)(iVar7 + 6) < 0) {
      FUN_80012050(~(int)*(short *)(iVar7 + 6),8);
    }
    if ((*param_1 & 0x10000) != 0) {
      return 0;
    }
    *param_1 = *param_1 | 0x10000;
    *(undefined2 *)((int)param_1 + 0x96) = 0xf0;
    break;
  case 4:
    iRam00000758 = iRam00000758 + -1;
    break;
  case 9:
    if ((int *)param_1[0x21] != param_3) {
      return 0;
    }
    param_1[0x21] = param_1[0x20];
  }
  return 0;
}

