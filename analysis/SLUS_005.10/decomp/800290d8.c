// addr: 0x800290d8  name: FUN_800290d8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_800290d8(void)

{
  undefined1 *puVar1;
  uint uVar2;
  short sVar3;
  short sVar4;
  short sVar5;
  short sVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  ushort uVar10;
  ushort uVar11;
  uint uVar12;
  uint uVar13;
  uint uVar14;
  short local_40;
  short local_3e;
  short local_3c;
  short local_38;
  short local_36;
  short local_34;
  undefined1 local_30 [4];
  undefined1 local_2c [4];
  undefined4 local_28;
  undefined4 local_24;
  
  iVar7 = DAT_8006f6f4;
  if (DAT_8006f6f4 < 0) {
    iVar7 = DAT_8006f6f4 + 0xffff;
  }
  iVar8 = DAT_8006f6fc;
  if (DAT_8006f6fc < 0) {
    iVar8 = DAT_8006f6fc + 0xffff;
  }
  if ((short)DAT_8006f6e8 < 1) {
    local_38 = (short)(-iRam000006d8 / 2) + -0x10;
  }
  else {
    local_38 = (short)(iRam000006d8 / 2) + 0x10;
  }
  local_40 = -local_38;
  iVar9 = iRam000006dc;
  if (-1 < DAT_8006f6e4._2_2_) {
    iVar9 = -iRam000006dc;
  }
  local_36 = (short)((uint)(iVar9 - (iVar9 >> 0x1f)) >> 1);
  local_3e = -local_36;
  local_34 = (short)iRam000006d4;
  local_3c = (short)iRam000006d4;
  SetRotMatrix((MATRIX *)&DAT_8006f6e0);
  FUN_8004316c(&local_40,&local_40);
  FUN_8004316c(&local_38,&local_38);
  iVar9 = -DAT_8006f6f4;
  local_40 = (short)((local_40 * 0x50) / iRam000006d4);
  local_3c = (short)((local_3c * 0x50) / iRam000006d4);
  local_38 = (short)((local_38 * 0x50) / iRam000006d4);
  local_34 = (short)((local_34 * 0x50) / iRam000006d4);
  if (iVar9 < 0) {
    iVar9 = iVar9 + 0xff;
  }
  DAT_1f800004 = iVar9 >> 8;
  iVar9 = -DAT_8006f6f8;
  if (iVar9 < 0) {
    iVar9 = iVar9 + 0xff;
  }
  DAT_1f800008 = iVar9 >> 8;
  _DAT_1f80000c = -DAT_8006f6fc;
  if (_DAT_1f80000c < 0) {
    _DAT_1f80000c = _DAT_1f80000c + 0xff;
  }
  _DAT_1f80000c = _DAT_1f80000c >> 8;
  SetRotMatrix((MATRIX *)&DAT_8006f680);
  gte_ldtr(0,0,0);
  SetColorMatrix((MATRIX *)&DAT_8005e994);
  SetLightMatrix((MATRIX *)&DAT_8005e974);
  SetBackColor((uint)bRam00000850,(uint)bRam00000851,(uint)bRam00000852);
  SetFarColor((uint)bRam000007fc,(uint)bRam000007fd,(uint)bRam000007fe);
  DAT_1f800000 = iRam0000060c + 0x100;
  DAT_1f800014 = &UNK_80092220 + iRam00000004 * 0x7e00;
  DAT_1f800018 = &UNK_8007a9a0 + iRam00000004 * 0xa000;
  DAT_1f80001c = &UNK_8008e9a0 + iRam00000004 * 0x340;
  sVar3 = (short)((uint)iVar7 >> 0x10);
  local_30._0_2_ = sVar3 + 2;
  if (local_34 < 1) {
    local_30._0_2_ = sVar3 + -1;
  }
  sVar4 = (short)((uint)iVar8 >> 0x10);
  if (local_38 < 0) {
    local_30._2_2_ = sVar4 + 2;
  }
  else {
    local_30._2_2_ = sVar4 + -1;
  }
  if (local_3c < 0) {
    local_2c._0_2_ = sVar3 + 2;
  }
  else {
    local_2c._0_2_ = sVar3 + -1;
  }
  if (local_40 < 1) {
    local_2c._2_2_ = sVar4 + -1;
  }
  else {
    local_2c._2_2_ = sVar4 + 2;
  }
  sVar5 = sVar3 + (local_40 >> 2);
  if (local_3c < 0) {
    sVar5 = sVar5 + 2;
  }
  sVar6 = sVar4 + (local_3c >> 2);
  if (0 < local_40) {
    sVar6 = sVar6 + 2;
  }
  local_28 = CONCAT22(sVar6,sVar5) & 0xfffefffe;
  sVar5 = sVar3 + (local_38 >> 2);
  if (0 < local_34) {
    sVar5 = sVar5 + 2;
  }
  sVar6 = sVar4 + (local_34 >> 2);
  if (local_38 < 0) {
    sVar6 = sVar6 + 2;
  }
  uVar12 = 1;
  local_24 = CONCAT22(sVar6,sVar5) & 0xfffefffe;
  SetFogNearFar(0x500,0x1000,iRam000006d4);
  FUN_800289d8(0,local_30);
  uVar14 = 1;
  uVar13 = 2;
  SetFogNearFar(0x2800,0x5000,iRam000006d4);
  do {
    puVar1 = local_2c + 3;
    uVar2 = (uint)puVar1 & 3;
    *(uint *)(puVar1 + -uVar2) =
         *(uint *)(puVar1 + -uVar2) & -1 << (uVar2 + 1) * 8 | local_28 >> (3 - uVar2) * 8;
    local_2c = (undefined1  [4])local_28;
    puVar1 = local_30 + 3;
    uVar2 = (uint)puVar1 & 3;
    *(uint *)(puVar1 + -uVar2) =
         *(uint *)(puVar1 + -uVar2) & -1 << (uVar2 + 1) * 8 | local_24 >> (3 - uVar2) * 8;
    local_30 = (undefined1  [4])local_24;
    uVar10 = sVar3 + (short)((int)local_40 >> (uVar14 & 0x1f));
    if (local_3c < 0) {
      uVar10 = uVar10 + (short)(1 << (uVar13 & 0x1f));
    }
    uVar11 = sVar4 + (short)((int)local_3c >> (uVar14 & 0x1f));
    if (0 < local_40) {
      uVar11 = uVar11 + (short)(1 << (uVar13 & 0x1f));
    }
    local_28 = CONCAT22(uVar11 & -(short)(2 << (uVar12 & 0x1f)),
                        uVar10 & -(short)(2 << (uVar12 & 0x1f)));
    uVar10 = sVar3 + (short)((int)local_38 >> (uVar14 & 0x1f));
    if (0 < local_34) {
      uVar10 = uVar10 + (short)(1 << (uVar13 & 0x1f));
    }
    uVar11 = sVar4 + (short)((int)local_34 >> (uVar14 & 0x1f));
    if (local_38 < 0) {
      uVar11 = uVar11 + (short)(1 << (uVar13 & 0x1f));
    }
    local_24 = CONCAT22(uVar11 & -(short)(2 << (uVar12 & 0x1f)),
                        uVar10 & -(short)(2 << (uVar12 & 0x1f)));
    FUN_800289d8(uVar12,local_30);
    uVar14 = uVar14 - 1;
    uVar12 = uVar12 + 1;
    uVar13 = uVar13 + 1;
  } while ((int)uVar12 < 3);
  iVar9 = (int)(DAT_1f800014 + iRam00000004 * -0x7e00 + 0x7ff6dde0) * -0x49249249 >> 2;
  iVar8 = (int)(DAT_1f800018 + iRam00000004 * -0xa000 + 0x7ff85660) * -0x33333333 >> 3;
  iVar7 = (int)(DAT_1f80001c + iRam00000004 * -0x340 + 0x7ff71660) * -0x3b13b13b >> 2;
  if (iRam00000808 < iVar9) {
    iRam00000808 = iVar9;
  }
  if (iRam00000820 < iVar8) {
    iRam00000820 = iVar8;
  }
  if (iRam00000824 < iVar7) {
    iRam00000824 = iVar7;
  }
  return;
}

