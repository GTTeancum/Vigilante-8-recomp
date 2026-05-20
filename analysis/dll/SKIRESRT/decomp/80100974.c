// addr: 0x80100974  name: FUN_80100974

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100974(uint *param_1,undefined4 param_2,int param_3)

{
  short sVar1;
  int *piVar2;
  char cVar3;
  ushort uVar4;
  int iVar5;
  uint uVar6;
  undefined4 uVar7;
  uint *puVar8;
  int *piVar9;
  int iVar10;
  int iVar11;
  uint *puVar12;
  uint *puVar13;
  uint *puVar14;
  uint *puVar15;
  int local_80;
  int local_70;
  undefined1 auStack_48 [20];
  int local_34;
  int local_30;
  int local_2c;
  
  switch(param_2) {
  case 0:
    sVar1 = *(short *)((int)param_1 + 0xa2);
    if (sVar1 != 0) {
      *(short *)((int)param_1 + 0xa2) = sVar1 + -1;
      if (sVar1 != 1) {
        return 0;
      }
      cVar3 = FUN_8004410c/*0x8004410c*/();
      *(char *)((int)param_1 + 5) = cVar3;
      param_3 = *(int *)(*(int *)(param_1[0x26] + 0x58) + 8);
      FUN_800443c8/*0x800443c8*/((int)cVar3,param_3,0,0);
    }
    uVar4 = (short)param_1[0x28] + 0x20;
    *(ushort *)(param_1 + 0x28) = uVar4;
    iVar5 = 0;
    puVar8 = param_1;
    if ((uVar4 & 0x7fff) == 0) {
      do {
        uVar4 = *(ushort *)(puVar8[0x26] + 10) | 0x2c;
        *(ushort *)(puVar8[0x26] + 10) = uVar4;
        if (uVar4 != 0) {
          FUN_8001bddc/*0x8001bddc*/(*(undefined4 *)(puVar8[0x26] + 0x30));
          uVar6 = puVar8[0x26];
          *(undefined2 *)(uVar6 + 10) = 0x2c;
          uVar7 = FUN_8001bda0/*0x8001bda0*/(*(undefined4 *)(uVar6 + 0x58),0x2c);
          *(undefined4 *)(puVar8[0x26] + 0x30) = uVar7;
        }
        iVar5 = iVar5 + 1;
        puVar8 = puVar8 + 1;
      } while (iVar5 < 2);
      *(undefined2 *)((int)param_1 + 0xa2) = 0x4b0;
      FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
      param_3 = 1;
      *(undefined1 *)((int)param_1 + 5) = 0;
    }
    if (param_3 == 0) {
      return 0;
    }
    iVar5 = FUN_800446dc/*0x800446dc*/(param_1[0x26] + 0x48,
                            (int)(((ushort)param_1[0x28] + 0x8000) * 0x10000) >> 0x10);
    iVar11 = FUN_800446dc/*0x800446dc*/(param_1[0x27] + 0x48);
    FUN_80044574/*0x80044574*/((int)*(char *)((int)param_1 + 5),iVar5 + iVar11);
  case 0x10:
    puVar14 = *(uint **)param_1[0x20];
    puVar12 = (uint *)param_1[_DAT_80065308 + 0x23];
    puVar13 = puVar12 + 6;
    puVar8 = (uint *)param_1[0x20];
    if (puVar14 != (uint *)0x0) {
      do {
        uVar6 = puVar8[2];
        iVar5 = (int)*(short *)(uVar6 + 0x42);
        if (iVar5 < 0) {
          iVar5 = -iVar5;
        }
        uVar7 = 0xffff8000;
        if (0x400 < iVar5) {
          uVar7 = 0xffff8001;
        }
        iVar11 = FUN_8001b038/*0x8001b038*/(uVar6,uVar7);
        iVar5 = (int)*(short *)(uVar6 + 0x42);
        uVar7 = 0xffff8000;
        if (iVar5 < 0) {
          iVar5 = -iVar5;
        }
        if (iVar5 < 0x401) {
          uVar7 = 0xffff8001;
        }
        iVar5 = FUN_8001b038/*0x8001b038*/(uVar6,uVar7);
        CompMatrixLV/*0x8004cf04*/(0x8006f680,uVar6 + 0x10,auStack_48);
        SetRotMatrix/*0x8004d344*/(auStack_48);
        setCopControlWord(2,0x2800,local_34 >> 8);
        setCopControlWord(2,0x3000,local_30 >> 8);
        setCopControlWord(2,0x3800,local_2c >> 8);
        setCopReg(2,0,(*(int *)(iVar11 + 4) >> 8 & 0xffffU) + (*(int *)(iVar11 + 8) >> 8) * 0x10000)
        ;
        setCopReg(2,0x800,*(int *)(iVar11 + 0xc) >> 8);
        copFunction(2,0x180001);
        setCopReg(2,0,(*(int *)(iVar5 + 4) >> 8 & 0xffffU) + (*(int *)(iVar5 + 8) >> 8) * 0x10000);
        setCopReg(2,0x800,*(int *)(iVar5 + 0xc) >> 8);
        getCopReg(2,9);
        getCopReg(2,10);
        iVar5 = getCopReg(2,0xb);
        copFunction(2,0x180001);
        getCopReg(2,9);
        getCopReg(2,10);
        iVar11 = getCopReg(2,0xb);
        if (puVar8 != (uint *)param_1[0x20]) {
          if ((0x80 < local_80) || (0x80 < iVar5)) {
            uVar6 = getCopReg(2,0xd);
            puVar13[-3] = uVar6;
            iVar10 = iVar5;
            if (iVar5 < local_80) {
              iVar10 = local_80;
            }
            puVar8 = (uint *)((iVar10 >> 3) * 4 + _DAT_80065910);
            uVar6 = *puVar8;
            *puVar8 = (uint)puVar12 & 0xffffff;
            *puVar12 = uVar6 | 0x3000000;
          }
          if ((0x80 < local_70) || (0x80 < iVar11)) {
            uVar6 = getCopReg(2,0xe);
            puVar13[1] = uVar6;
            iVar10 = iVar11;
            if (iVar11 < local_70) {
              iVar10 = local_70;
            }
            puVar8 = (uint *)((iVar10 >> 3) * 4 + _DAT_80065910);
            uVar6 = *puVar8;
            *puVar8 = (uint)(puVar12 + 4) & 0xffffff;
            puVar12[4] = uVar6 | 0x3000000;
          }
          puVar13 = puVar13 + 8;
          puVar12 = puVar12 + 8;
        }
        uVar6 = getCopReg(2,0xd);
        puVar13[-4] = uVar6;
        uVar6 = getCopReg(2,0xe);
        *puVar13 = uVar6;
        puVar15 = (uint *)*puVar14;
        puVar8 = puVar14;
        puVar14 = puVar15;
        local_80 = iVar5;
        local_70 = iVar11;
      } while (puVar15 != (uint *)0x0);
switchD_801009c8_caseD_11:
      func_0x80022120(param_1);
      goto switchD_801009c8_caseD_1;
    }
    break;
  case 1:
switchD_801009c8_caseD_1:
    piVar9 = (int *)param_1[0x20];
    iVar11 = *(int *)(piVar9[2] + 0x50);
    iVar5 = *(int *)(*(int *)(param_1[0x22] + 8) + 0x50) - iVar11;
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0xff;
    }
    iVar10 = 0;
    for (piVar2 = (int *)*piVar9; piVar2 != (int *)0x0; piVar2 = (int *)*piVar2) {
      piVar9[3] = ((*(int *)(piVar9[2] + 0x50) - iVar11) * 0x70) / (iVar5 >> 8);
      iVar10 = iVar10 + 1;
      piVar9 = piVar2;
    }
    iVar10 = iVar10 << 1;
    uVar6 = Heap_CallocOrRetry/*0x8001178c*/(0x10,iVar10);
    param_1[0x23] = uVar6;
    uVar6 = Heap_CallocOrRetry/*0x8001178c*/(0x10,iVar10);
    iVar5 = 0;
    param_1[0x24] = uVar6;
    if (0 < iVar10) {
      do {
        iVar11 = iVar5 * 0x10;
        *(undefined1 *)(iVar11 + param_1[0x23] + 3) = 3;
        *(undefined1 *)(iVar11 + param_1[0x23] + 7) = 0x40;
        *(undefined1 *)(iVar11 + param_1[0x24] + 3) = 3;
        iVar5 = iVar5 + 1;
        *(undefined1 *)(iVar11 + param_1[0x24] + 7) = 0x40;
      } while (iVar5 < iVar10);
    }
    *(undefined2 *)((int)param_1 + 0xa2) = 0x4b0;
    *param_1 = *param_1 | 0x80;
    uVar7 = FUN_8001ffd4/*0x8001ffd4*/(0x80065a50,0x100);
    iVar5 = func_0x8003d080(0x7f000000,uVar7);
    _DAT_80065a10 = (uint)(iVar5 != 0);
  case 2:
    func_0x80023d00();
    FUN_80020890/*0x80020890*/(param_1,0xf0);
  case 7:
    param_1 = (uint *)FUN_8001d470/*0x8001d470*/(0xa4);
    param_1[0x20] = (uint)(param_1 + 0x21);
    param_1[0x21] = 0;
    param_1[0x22] = (uint)(param_1 + 0x20);
  case 4:
    Heap_Free/*0x80045088*/(param_1[0x23]);
    Heap_Free/*0x80045088*/(param_1[0x24]);
    func_0x8001fde8(param_1 + 0x20);
    break;
  default:
    break;
  case 0x11:
    goto switchD_801009c8_caseD_11;
  }
  return 0;
}

