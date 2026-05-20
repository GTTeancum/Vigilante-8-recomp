// addr: 0x8002d054  name: FUN_8002d054

void FUN_8002d054(uint *param_1)

{
  byte bVar1;
  short sVar2;
  int iVar3;
  undefined4 uVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  undefined4 uVar9;
  uint uVar10;
  uint unaff_s1;
  int iVar11;
  uint *puVar12;
  
  iVar3 = *(char *)((int)param_1 + 5) * 0x10;
  if ((*param_1 & 0x8000000) == 0) {
    if ((*param_1 & 0x100000) == 0) {
      iVar7 = 0x300;
      if (0 < *(short *)((int)param_1 + 0xa6)) {
        iVar7 = 0x800;
      }
    }
    else {
      iVar11 = param_1[0x23] *
               (uint)*(ushort *)(&DAT_8005ec68 + (*(char *)((int)param_1 + 0xb2) + 1) * 2);
      if (iVar11 < 0) {
        iVar11 = iVar11 + 0xfff;
      }
      iVar11 = iVar11 >> 0xc;
      if ((iVar11 < 0x300) && (1 < *(char *)((int)param_1 + 0xb2))) {
        *(char *)((int)param_1 + 0xb2) = *(char *)((int)param_1 + 0xb2) + -1;
      }
      if (0x800 < iVar11) {
        bVar1 = *(byte *)((int)param_1 + 0xb2);
        if ((bVar1 < 3) && (-1 < (int)((uint)bVar1 << 0x18))) {
          *(byte *)((int)param_1 + 0xb2) = bVar1 + 1;
        }
      }
      iVar7 = 0x300;
      if (0x300 < iVar11) {
        iVar7 = iVar11;
      }
    }
    iVar7 = iVar7 - (short)param_1[0x35];
    iVar11 = -0x80;
    if (-0x80 < iVar7) {
      iVar11 = iVar7;
    }
    sVar2 = 0x80;
    if (iVar11 < 0x80) {
      sVar2 = (short)iVar11;
    }
    sVar2 = (short)param_1[0x35] + sVar2;
    *(short *)(param_1 + 0x35) = sVar2;
    *(short *)(iVar3 + 0x1f801bf4) = sVar2;
    if (((&DAT_80065c30)[~(int)*(short *)((int)param_1 + 6) * 6] & 0x100) == 0) {
      iVar11 = *(short *)((int)param_1 + 0xd6) + -0x80;
      iVar7 = 0x800;
      if (0x800 < iVar11) {
        iVar7 = iVar11;
      }
      *(short *)((int)param_1 + 0xd6) = (short)iVar7;
    }
    else {
      iVar11 = *(short *)((int)param_1 + 0xd6) + 0x80;
      iVar7 = 0x1000;
      if (iVar11 < 0x1000) {
        iVar7 = iVar11;
      }
      *(short *)((int)param_1 + 0xd6) = (short)iVar7;
    }
    unaff_s1 = FUN_800446dc(param_1 + 9);
    iVar7 = (unaff_s1 & 0xffff) * (int)*(short *)((int)param_1 + 0xd6);
    if (iVar7 < 0) {
      iVar7 = iVar7 + 0xfff;
    }
    *(short *)(iVar3 + 0x1f801bf0) = (short)(iVar7 >> 0xc);
    iVar7 = ((int)unaff_s1 >> 0x10) * (int)*(short *)((int)param_1 + 0xd6);
    if (iVar7 < 0) {
      iVar7 = iVar7 + 0xfff;
    }
    *(short *)(iVar3 + 0x1f801bf2) = (short)(iVar7 >> 0xc);
  }
  else {
    uVar4 = FUN_800446dc(param_1 + 9);
    FUN_80044574((int)*(char *)((int)param_1 + 5),uVar4);
  }
  if ((*param_1 & 0x400000) != 0) {
    uVar4 = FUN_8004410c();
    uVar9 = 0x1e;
    if (0 < (short)param_1[6]) {
      uVar9 = 0x1d;
    }
    FUN_8004483c(uVar4,uRam000005f8,uVar9,param_1 + 9);
    FUN_80012068(~(int)*(short *)((int)param_1 + 6),0xc0,0,0x40);
  }
  uVar10 = *param_1;
  if (((uVar10 & 0x100000) != 0) && (0xbeb < (int)param_1[0x23])) {
    uVar5 = param_1[0x20];
    if ((int)uVar5 < 0) {
      uVar5 = uVar5 + 0x7f;
    }
    uVar8 = param_1[0x21];
    if ((int)uVar8 < 0) {
      uVar8 = uVar8 + 0x7f;
    }
    uVar6 = param_1[0x22];
    if ((int)uVar6 < 0) {
      uVar6 = uVar6 + 0x7f;
    }
    iVar3 = ((int)uVar5 >> 7) * (int)(short)param_1[5] +
            ((int)uVar8 >> 7) * (int)*(short *)((int)param_1 + 0x1a) +
            ((int)uVar6 >> 7) * (int)(short)param_1[8];
    if (iVar3 < 0) {
      iVar3 = -iVar3;
    }
    if (iVar3 < (int)(param_1[0x23] * 0xc00)) {
      if ((uVar10 & 0x80000) == 0) {
        *param_1 = uVar10 | 0x80000;
        uVar4 = FUN_8004410c();
        uVar10 = FUN_80017160();
        uVar9 = 0x1a;
        if ((uVar10 & 1) != 0) {
          uVar9 = 0x19;
        }
        FUN_8004483c(uVar4,uRam000005f8,uVar9,param_1 + 9);
      }
      goto LAB_8002d3dc;
    }
  }
  *param_1 = *param_1 & 0xfff7ffff;
LAB_8002d3dc:
  if (*(byte *)((int)param_1 + 0xd3) != 0) {
    iVar3 = (uint)*(byte *)((int)param_1 + 0xd3) * 0x10;
    puVar12 = (uint *)(iVar3 + 0x1f801bf0);
    iVar7 = (int)param_1[0x23] / 2;
    if (iVar7 < 0x300) {
      *puVar12 = 0;
    }
    else {
      iVar11 = 0xc00;
      if (iVar7 < 0xc00) {
        iVar11 = iVar7;
      }
      *(short *)(iVar3 + 0x1f801bf4) = (short)iVar11;
      *puVar12 = unaff_s1;
    }
  }
  return;
}

