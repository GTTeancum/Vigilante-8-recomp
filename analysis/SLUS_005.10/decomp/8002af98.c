// addr: 0x8002af98  name: FUN_8002af98

void FUN_8002af98(int param_1,int param_2,uint *param_3)

{
  undefined4 *puVar1;
  char cVar2;
  undefined2 uVar3;
  short sVar4;
  int iVar5;
  uint uVar6;
  uint *puVar7;
  short *psVar8;
  void *p;
  int iVar9;
  int iVar10;
  short sVar11;
  int iVar12;
  uint uVar13;
  short sVar14;
  short *psVar15;
  char acStack_30 [8];
  
  iVar12 = 0;
  iVar5 = param_2 * 0x14;
  psVar15 = (short *)(&DAT_8005e9e4 + iVar5);
  iVar9 = *(int *)(param_1 + (*(byte *)(param_1 + 0xb3) + 9) * 4 + 0xec);
  iVar10 = iRam00000004 * 0x74;
  psVar8 = psVar15;
  do {
    *(short *)((int)&DAT_800a28b4 + iVar10) = *psVar8;
    iVar12 = iVar12 + 1;
    *(short *)((int)&DAT_800a28b4 + iVar10 + 2) = psVar8[1];
    psVar8 = psVar8 + 2;
    iVar10 = iVar10 + 0x1c;
  } while (iVar12 < 4);
  if (iVar9 != 0) {
    sprintf(acStack_30,"%02i",(uint)*(ushort *)(iVar9 + 0xc));
    if (*(code **)(iVar9 + 100) == (code *)0x0) {
      cVar2 = '\0';
    }
    else {
      cVar2 = (**(code **)(iVar9 + 100))(iVar9,0xd,0);
    }
    *(char *)((int)&DAT_800a2fcc + iRam00000004 * 0x1c + 1) =
         (char)((ushort)uRam000008aa >> 8) + cVar2 * '\x10';
    iVar10 = iRam00000004;
    iVar9 = iRam00000004 * 0x1c;
    *(short *)(&DAT_800a2fc8 + iRam00000004 * 7) = *(short *)(&DAT_8005e9ec + iVar5) + 2;
    *(short *)((int)&DAT_800a2fc8 + iVar9 + 2) = *(short *)(&DAT_8005e9ee + iVar5) + 0xc;
    AddPrim(param_3,&DAT_800a2fb8 + iVar10 * 7);
    FUN_80019c0c(uRam0000087c,acStack_30,*(short *)(&DAT_8005e9ec + iVar5) + 0x34,
                 *(short *)(&DAT_8005e9ee + iVar5) + 0xc,param_3);
  }
  uVar13 = 0;
  sVar11 = 1;
  iVar10 = 0;
  iVar9 = 0x24;
  do {
    if (*(int *)(param_1 + iVar9 + 0xec) != 0) {
      puVar1 = &DAT_800a2ad8;
      if (uVar13 == *(byte *)(param_1 + 0xb3)) {
        puVar1 = &DAT_800a2a30;
      }
      p = (void *)(iVar10 + (int)puVar1);
      if (iRam00000004 != 0) {
        p = (void *)((int)p + 0x1c);
      }
      *(short *)((int)p + 0x10) = *(short *)(&DAT_8005e9ec + iVar5) + sVar11;
      *(short *)((int)p + 0x12) = *(short *)(&DAT_8005e9ee + iVar5) + -2;
      AddPrim(param_3,p);
    }
    sVar11 = sVar11 + 0x10;
    iVar10 = iVar10 + 0x38;
    uVar13 = uVar13 + 1;
    iVar9 = iVar9 + 4;
  } while ((int)uVar13 < 3);
  sVar14 = 0;
  iVar9 = 0;
  sVar11 = 0;
  iVar10 = param_1;
  do {
    if (*(short *)(iVar10 + 0x11c) != 0) {
      if (param_2 < 4) {
        iVar12 = iVar9 + iRam00000004 * 0x1c;
        *(short *)((int)&DAT_800a2998 + iVar12) = *psVar15 - (sVar11 + 0x18);
        sVar4 = *(short *)(&DAT_8005e9e6 + iVar5);
      }
      else {
        if (param_2 == 4) {
          uVar3 = 8;
        }
        else {
          uVar3 = 0x80;
        }
        *(undefined2 *)((int)&DAT_800a2998 + iVar9 + iRam00000004 * 0x1c) = uVar3;
        sVar4 = sVar14 * 0x10 + 0x50;
        iVar12 = iVar9 + iRam00000004 * 0x1c;
      }
      sVar11 = sVar11 + 0x18;
      sVar14 = sVar14 + 1;
      *(short *)((int)&DAT_800a2998 + iVar12 + 2) = sVar4;
      if ((300 < *(ushort *)(iVar10 + 0x11c)) || (0x14 < (*(ushort *)(iVar10 + 0x11c) & 0x1f))) {
        AddPrim(param_3,(void *)((int)&DAT_800a2988 + iVar9 + iRam00000004 * 0x1c));
      }
    }
    iVar10 = iVar10 + 2;
    iVar9 = iVar9 + 0x38;
  } while (iVar10 < param_1 + 6);
  if (*(short *)(param_1 + 0xc) != 0) {
    if (*(char *)(param_1 + 0xd0) == '\f') {
      uVar13 = ((uint)*(ushort *)(param_1 + 0xc) * 0x27) / (uint)*(ushort *)(param_1 + 0xe) + 3;
    }
    else {
      uVar6 = (((uint)*(ushort *)(*(int *)(param_1 + 0xec) + 0xc) +
                (uint)*(ushort *)(*(int *)(param_1 + 0xf0) + 0xc) +
               (uint)*(ushort *)(*(int *)(param_1 + 0xf4) + 0xc)) * 0x27) /
              ((uint)*(ushort *)(param_1 + 0xc) * 3) + 3;
      uVar13 = 0x2a;
      if (uVar6 < 0x2a) {
        uVar13 = uVar6;
      }
    }
    if ((10 < uVar13) || ((uRam0000000c & 0x1f) < 0x14)) {
      iVar10 = iRam00000004 * 0x1c;
      *(short *)(&DAT_800a2b90 + iRam00000004 * 7) = *psVar15 + 4;
      cVar2 = cRam00000880;
      *(short *)((int)&DAT_800a2b90 + iVar10 + 2) =
           *(short *)(&DAT_8005e9e6 + iVar5) - ((short)uVar13 + -0x2e);
      *(char *)((int)&DAT_800a2b94 + iVar10 + 1) = cVar2 - ((char)uVar13 + -0x2a);
      iVar10 = iRam00000004 * 7;
      *(short *)((int)&DAT_800a2b98 + iRam00000004 * 0x1c + 2) = (short)uVar13;
      AddPrim(param_3,&DAT_800a2b80 + iVar10);
    }
  }
  FUN_8002ad30(param_1,param_3,*(short *)(&DAT_8005e9e8 + iVar5) + 0x1b,
               *(short *)(&DAT_8005e9ea + iVar5) + 0x1b);
  iVar10 = *(int *)(param_1 + 0xe4);
  if ((iVar10 != 0) && (*(int *)(iVar10 + 0x7c) != 0)) {
    if (*(char *)(iVar10 + 4) == '\x02') {
      if (*(short *)(iVar10 + 0xc) == 0) {
        sVar11 = 0x20;
      }
      else if (*(char *)(iVar10 + 0xd0) == '\f') {
        sVar11 = 0x20 - (short)(((uint)*(ushort *)(iVar10 + 0xc) << 5) /
                               (uint)*(ushort *)(iVar10 + 0xe));
      }
      else {
        sVar11 = 0x20 - (short)((((uint)*(ushort *)(*(int *)(iVar10 + 0xec) + 0xc) +
                                  (uint)*(ushort *)(*(int *)(iVar10 + 0xf0) + 0xc) +
                                 (uint)*(ushort *)(*(int *)(iVar10 + 0xf4) + 0xc)) * 0x20) /
                               ((uint)*(ushort *)(iVar10 + 0xc) * 3));
      }
    }
    else {
      sVar11 = 0;
    }
    FUN_80018cb8(&DAT_800a2828 + iRam00000004 * 0xf,*(int *)(iVar10 + 0x7c));
    iVar9 = iRam00000004;
    iVar12 = iRam00000004 * 0x3c;
    iVar10 = iRam00000004 * 0xf;
    *(short *)(&DAT_800a2838 + iRam00000004 * 0xf) = *(short *)(&DAT_8005e9f0 + iVar5) + 3;
    *(short *)((int)&DAT_800a2838 + iVar12 + 2) = *(short *)(&DAT_8005e9f2 + iVar5) + 2;
    sVar14 = *(short *)(&DAT_8005e9f2 + iVar5);
    *(short *)(&DAT_800a285e + iVar12) = sVar14 + 0x12;
    *(short *)((int)&DAT_800a284c + iVar12 + 2) = sVar14 + 0x12;
    sVar14 = *(short *)(&DAT_8005e9f0 + iVar5);
    *(short *)(&DAT_800a2860 + iVar12) = 0x20 - sVar11;
    *(short *)(&DAT_800a2850 + iVar9 * 0xf) = sVar11;
    *(short *)(&DAT_800a285c + iVar12) = sVar14 + 7;
    *(short *)(&DAT_800a284c + iVar9 * 0xf) = sVar14 + 7 + (0x20 - sVar11);
    AddPrim(param_3,&DAT_800a2828 + iVar10);
  }
  uVar13 = *param_3;
  puVar7 = (uint *)(&DAT_800a28a0)[iRam00000004 * 0x1d];
  *param_3 = (uint)(&DAT_800a28a4 + iRam00000004 * 0x1d) & 0xffffff;
  *puVar7 = (uint)*(byte *)((int)puVar7 + 3) << 0x18 | uVar13;
  return;
}

