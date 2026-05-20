// addr: 0x801006d4  name: FUN_801006d4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801006d4(uint *param_1,int param_2)

{
  char cVar1;
  byte bVar2;
  char cVar3;
  undefined4 uVar4;
  int iVar5;
  short *psVar6;
  uint uVar7;
  short sVar8;
  int iVar9;
  uint *puVar10;
  uint uVar11;
  uint uVar12;
  uint uVar13;
  uint *puVar14;
  
  if ((param_2 != 0) && (param_2 == 2)) goto LAB_80100a74;
  puVar14 = (uint *)param_1[0x1e];
  iVar5 = ~(int)*(short *)((int)param_1 + 6) * 0x18;
  psVar6 = (short *)(iVar5 + -0x7ff9a3d8);
  uVar11 = *(uint *)(iVar5 + -0x7ff9a3d0);
  puVar10 = param_1 + 0x29;
  if ((*param_1 & 1) != 0) {
    uVar7 = param_1[0x20];
    if ((int)uVar7 < 0) {
      uVar7 = uVar7 + 0x7f;
    }
    puVar10 = (uint *)param_1[0x21];
    param_1[9] = param_1[9] + ((int)uVar7 >> 7);
    if ((int)puVar10 < 0) {
      puVar10 = (uint *)((int)puVar10 + 0x7f);
    }
    uVar7 = param_1[0x22];
    param_1[10] = param_1[10] + ((int)puVar10 >> 7);
    if ((int)uVar7 < 0) {
      uVar7 = uVar7 + 0x7f;
    }
    psVar6 = (short *)((int)uVar7 >> 7);
    param_1[0xb] = param_1[0xb] + (int)psVar6;
  }
  sVar8 = *psVar6;
  if (sVar8 == 3) {
LAB_80100854:
    bVar2 = *(byte *)(psVar6 + 8);
    *(ushort *)(param_1 + 0x29) = (bVar2 - 0x80) * 5;
    uVar7 = (uint)*(byte *)(bVar2 - 0x70);
    iVar5 = uVar7 - 0x80;
    if (iVar5 < 0) {
      iVar5 = uVar7 - 0x7d;
    }
    iVar9 = (int)(short)*puVar10 + (iVar5 >> 2);
    iVar5 = -0x2aa;
    if ((-0x2ab < iVar9) && (iVar5 = 0x2aa, iVar9 < 0x2ab)) {
      iVar5 = iVar9;
    }
    iVar9 = (int)(short)*puVar10;
    if (iVar9 < 0) {
      iVar9 = iVar9 + 0xf;
    }
    *(short *)puVar10 = (short)iVar5 - (short)(iVar9 >> 4);
  }
  else {
    if (sVar8 < 4) {
      uVar7 = uVar11 & 0x800;
      if (sVar8 != 2) {
        sVar8 = 1;
        goto LAB_801007c0;
      }
LAB_801007d4:
      if (uVar7 != 0) {
        iVar9 = (short)param_1[0x29] + -0x10;
        iVar5 = -0x2aa;
        if (-0x2aa < iVar9) {
          iVar5 = iVar9;
        }
        *(short *)(param_1 + 0x29) = (short)iVar5;
      }
      if ((uVar11 & 0x1000) != 0) {
        *(undefined2 *)(param_1 + 0x29) = 1;
      }
      iVar5 = (int)(short)param_1[0x29];
      psVar6 = (short *)(uint)(ushort)param_1[0x29];
      if (iVar5 < 0) {
        iVar5 = iVar5 + 0xf;
      }
      *(ushort *)(param_1 + 0x29) = (ushort)param_1[0x29] - (short)(iVar5 >> 4);
      goto LAB_80100854;
    }
LAB_801007c0:
    uVar7 = (uint)(sVar8 < 6);
    if (uVar7 != 0) goto LAB_801007d4;
  }
  if ((char)puVar14[2] < '\x04') {
    iVar5 = (int)(short)*puVar10;
    if (iVar5 < 0) {
      iVar5 = -iVar5;
    }
    if (0x2a < iVar5) {
      *(undefined1 *)(puVar14 + 2) = 0xff;
    }
  }
  if ((char)puVar14[2] < '\0') {
    iVar5 = (int)(short)*puVar10;
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0x1f;
    }
    *(short *)((int)puVar14 + 0x42) = *(short *)((int)puVar14 + 0x42) + (short)(iVar5 >> 5);
  }
  uVar7 = puVar14[5];
  uVar12 = puVar14[6];
  uVar13 = puVar14[7];
  param_1[4] = puVar14[4];
  param_1[5] = uVar7;
  param_1[6] = uVar12;
  param_1[7] = uVar13;
  uVar7 = puVar14[9];
  uVar12 = puVar14[10];
  uVar13 = puVar14[0xb];
  param_1[8] = puVar14[8];
  param_1[9] = uVar7;
  param_1[10] = uVar12;
  param_1[0xb] = uVar13;
  uVar7 = puVar14[0x13];
  uVar12 = puVar14[0x14];
  param_1[0x12] = puVar14[0x12];
  param_1[0x13] = uVar7;
  param_1[0x14] = uVar12;
  cVar1 = (char)puVar14[2];
  if (cVar1 < '\x05') {
    if ((*param_1 & 1) != 0) {
      return 0;
    }
    if (cVar1 == '\x04') {
      return 0;
    }
    if ((cVar1 != '\x02') && ((uVar11 & 0x1000000) == 0)) {
      return 0;
    }
  }
  uVar4 = FUN_8004410c/*0x8004410c*/();
  FUN_8004483c/*0x8004483c*/(uVar4,_DAT_800658fc,0x14,puVar14 + 0x12);
  puVar14[0x24] = 0;
  *puVar14 = *puVar14 | 0x20;
  FUN_80020890/*0x80020890*/(puVar14,0x78);
  func_0x80031294(param_1);
  func_0x8003d8c4(param_1[0x38]);
  *param_1 = *param_1 & 0xfeffffdd | 8;
  cVar1 = (char)puVar14[2];
  cVar3 = '\x02';
  if ((cVar1 == '\x02') || (cVar3 = cVar1 < '\x05', cVar1 < '\0')) {
    *(undefined1 *)(puVar14 + 2) = 0;
  }
  if (cVar3 != '\0') {
    return 0;
  }
  param_1[0x21] = 0xffffee1f;
LAB_80100a74:
  *param_1 = *param_1 | 2;
  return 0;
}

