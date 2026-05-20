// addr: 0x80104d1c  name: FUN_80104d1c

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_80104d1c(undefined4 param_1,int param_2)

{
  ushort uVar1;
  undefined2 uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int *piVar6;
  int iVar7;
  undefined4 uVar8;
  short *psVar9;
  int *piVar10;
  short sVar11;
  short *psVar12;
  uint uVar13;
  undefined4 local_res0 [4];
  undefined1 auStack_40 [32];
  
  local_res0[0] = param_1;
  iVar3 = func_0x800224ec(local_res0);
  iVar4 = func_0x800224ec(local_res0);
  uVar1 = func_0x8002249c(local_res0);
  iVar5 = func_0x8002249c(local_res0);
  piVar6 = (int *)Heap_AllocOrRetry/*0x800116f4*/(iVar5 * 4 + 0x1c);
  *(ushort *)(piVar6 + 4) = uVar1;
  *piVar6 = iVar3;
  piVar6[2] = iVar4;
  *(short *)((int)piVar6 + 0x12) = (short)iVar5;
  if ((uVar1 & 2) != 0) {
    iVar7 = func_0x800224ec(local_res0);
    piVar6[1] = iVar7 + -0x100000;
    param_2 = param_2 + -4;
  }
  iVar7 = Terrain_HeightAt/*0x80025400*/(iVar3,iVar4);
  piVar6[1] = iVar7;
  iVar7 = 0;
  piVar10 = piVar6;
  if (0 < iVar5) {
    do {
      piVar10[7] = 0;
      iVar7 = iVar7 + 1;
      piVar10 = piVar10 + 1;
    } while (iVar7 < iVar5);
  }
  if (10 < param_2) {
    uVar8 = FUN_80016da8/*0x80016da8*/(auStack_40);
    SetRotMatrix/*0x8004d344*/(uVar8);
    setCopControlWord(2,0x2800,iVar3);
    setCopControlWord(2,0x3000,piVar6[1]);
    setCopControlWord(2,0x3800,iVar4);
    iVar3 = func_0x800224b4(local_res0);
    piVar6[3] = *(int *)(&DAT_800737a0 + (iVar3 + 0x12) * 4);
    uVar2 = func_0x800224b4(local_res0);
    *(undefined2 *)(piVar6 + 5) = uVar2;
    uVar2 = func_0x800224b4(local_res0);
    *(undefined2 *)((int)piVar6 + 0x16) = uVar2;
    iVar3 = FUN_8001bda0/*0x8001bda0*/(piVar6[3],(short)piVar6[5]);
    piVar6[6] = iVar3;
    *(undefined2 *)(iVar3 + 0x28) = 0x10;
    iVar3 = piVar6[6];
    psVar12 = *(short **)(iVar3 + 8);
    uVar13 = 0x10 - *(ushort *)(iVar3 + 0x26);
    psVar9 = (short *)Heap_AllocOrRetry/*0x800116f4*/(*(int *)(iVar3 + 4) << 3);
    *(short **)(piVar6[6] + 8) = psVar9;
    iVar3 = 0;
    if (0 < *(int *)(piVar6[6] + 4)) {
      do {
        iVar4 = (*(ushort *)((int)piVar6 + 0x16) & 0xfff) * 4;
        iVar4 = (int)*(short *)(iVar4 + -0x7ff9f84a) * (int)*psVar12 +
                (int)*(short *)(iVar4 + -0x7ff9f84c) * (int)psVar12[2];
        if (iVar4 < 0) {
          iVar4 = iVar4 + 0xfff;
        }
        *psVar9 = (short)(iVar4 >> 0xc);
        iVar4 = (*(ushort *)((int)piVar6 + 0x16) & 0xfff) * 4;
        iVar4 = -(int)*(short *)(iVar4 + -0x7ff9f84c) * (int)*psVar12 +
                (int)*(short *)(iVar4 + -0x7ff9f84a) * (int)psVar12[2];
        if (iVar4 < 0) {
          iVar4 = iVar4 + 0xfff;
        }
        sVar11 = (short)(iVar4 >> 0xc);
        psVar9[2] = sVar11;
        iVar4 = Terrain_HeightAt/*0x80025400*/(*piVar6 + ((int)*psVar9 << (uVar13 & 0x1f)),
                                piVar6[2] + ((int)sVar11 << (uVar13 & 0x1f)));
        psVar9[1] = (short)(iVar4 - piVar6[1] >> (uVar13 & 0x1f));
        psVar12 = psVar12 + 4;
        iVar3 = iVar3 + 1;
        psVar9 = psVar9 + 4;
      } while (iVar3 < *(int *)(piVar6[6] + 4));
    }
    *(ushort *)piVar6[6] = *(ushort *)piVar6[6] & 0xfffe | 4;
  }
  piVar6[6] = 0;
  iVar3 = 0;
  iVar4 = *_DAT_80065bd8;
  piVar10 = _DAT_80065bd8;
  while (iVar4 != 0) {
    piVar10 = piVar10 + 1;
    iVar3 = iVar3 + 1;
    iVar4 = *piVar10;
  }
  _DAT_80065bd8[iVar3] = (int)piVar6;
  return;
}

