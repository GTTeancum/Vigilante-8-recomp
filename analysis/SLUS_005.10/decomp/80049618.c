// addr: 0x80049618  name: BIOS_OBJ_64

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 BIOS_OBJ_64(byte *param_1)

{
  undefined1 uVar1;
  undefined4 uVar2;
  byte *pbVar3;
  undefined1 *puVar4;
  undefined4 *puVar5;
  int iVar6;
  int iVar7;
  uint unaff_s1;
  byte in_stack_00000010;
  byte bStack00000018;
  byte bStack00000019;
  
  while (iVar6 = 0, in_stack_00000010 != (*param_1 & 7)) {
    in_stack_00000010 = *param_1 & 7;
  }
  do {
    pbVar3 = (byte *)((int)&stack0x00000018 + iVar6);
    iVar7 = iVar6;
    if ((CDROM_REG0 & 0x20) == 0) break;
    iVar6 = iVar6 + 1;
    *pbVar3 = CDROM_REG1;
    iVar7 = iVar6;
  } while (iVar6 < 8);
  for (; iVar6 < 8; iVar6 = iVar6 + 1) {
    *(undefined1 *)((int)&stack0x00000018 + iVar6) = 0;
  }
  CDROM_REG0 = 1;
  CDROM_REG3 = 7;
  CDROM_REG2 = 7;
  if ((in_stack_00000010 != 3) || (*(int *)(&DAT_80060244 + (uint)DAT_8006009d * 4) != 0)) {
    if (((_DAT_8006008c & 0x10) == 0) && ((bStack00000018 & 0x10) != 0)) {
      DAT_80060094 = DAT_80060094 + 1;
    }
    _DAT_8006008c = (uint)bStack00000018;
    DAT_80060090 = (uint)bStack00000019;
    unaff_s1 = _DAT_8006008c & 0x1d;
  }
  if (((in_stack_00000010 == 5) && (0 < DAT_80060088)) && (printf("DiskError: "), 0 < DAT_80060088))
  {
    printf("com=%s,code=(%02x:%02x)\n",(&PTR_s_CdlSync_800600a4)[DAT_8006009d],_DAT_8006008c,
           DAT_80060090);
  }
  switch(in_stack_00000010) {
  case 1:
    if ((unaff_s1 != 0) && (iVar7 == 1)) {
      unaff_s1 = 0;
    }
    DAT_8006035d = 1;
    if (unaff_s1 != 0) {
      DAT_8006035d = 5;
    }
    puVar4 = &DAT_800a3248;
    puVar5 = (undefined4 *)&stack0x00000018;
    iVar6 = 7;
    do {
      uVar1 = *(undefined1 *)puVar5;
      puVar5 = (undefined4 *)((int)puVar5 + 1);
      iVar6 = iVar6 + -1;
      *puVar4 = uVar1;
      puVar4 = puVar4 + 1;
    } while (iVar6 != -1);
    CDROM_REG0 = 0;
    CDROM_REG3 = 0;
    uVar2 = BIOS_OBJ_548();
    return uVar2;
  case 2:
    DAT_8006035c = 2;
    if (unaff_s1 != 0) {
      DAT_8006035c = 5;
    }
    puVar4 = &DAT_800a3240;
    puVar5 = (undefined4 *)&stack0x00000018;
    iVar6 = 7;
    do {
      uVar1 = *(undefined1 *)puVar5;
      puVar5 = (undefined4 *)((int)puVar5 + 1);
      iVar6 = iVar6 + -1;
      *puVar4 = uVar1;
      puVar4 = puVar4 + 1;
    } while (iVar6 != -1);
    uVar2 = BIOS_OBJ_548();
    return uVar2;
  case 3:
    break;
  case 4:
    puVar4 = &DAT_800a3250;
    DAT_8006035e = 4;
    puVar5 = (undefined4 *)&stack0x00000018;
    DAT_8006035d = 4;
    iVar6 = 7;
    do {
      uVar1 = *(undefined1 *)puVar5;
      puVar5 = (undefined4 *)((int)puVar5 + 1);
      iVar6 = iVar6 + -1;
      *puVar4 = uVar1;
      puVar4 = puVar4 + 1;
    } while (iVar6 != -1);
    puVar4 = &DAT_800a3248;
    puVar5 = (undefined4 *)&stack0x00000018;
    iVar6 = 7;
    do {
      uVar1 = *(undefined1 *)puVar5;
      puVar5 = (undefined4 *)((int)puVar5 + 1);
      iVar6 = iVar6 + -1;
      *puVar4 = uVar1;
      puVar4 = puVar4 + 1;
    } while (iVar6 != -1);
    uVar2 = BIOS_OBJ_548();
    return uVar2;
  case 5:
    puVar4 = &DAT_800a3240;
    DAT_8006035d = 5;
    puVar5 = (undefined4 *)&stack0x00000018;
    DAT_8006035c = 5;
    iVar6 = 7;
    do {
      uVar1 = *(undefined1 *)puVar5;
      puVar5 = (undefined4 *)((int)puVar5 + 1);
      iVar6 = iVar6 + -1;
      *puVar4 = uVar1;
      puVar4 = puVar4 + 1;
    } while (iVar6 != -1);
    puVar4 = &DAT_800a3248;
    puVar5 = (undefined4 *)&stack0x00000018;
    iVar6 = 7;
    do {
      uVar1 = *(undefined1 *)puVar5;
      puVar5 = (undefined4 *)((int)puVar5 + 1);
      iVar6 = iVar6 + -1;
      *puVar4 = uVar1;
      puVar4 = puVar4 + 1;
    } while (iVar6 != -1);
    uVar2 = BIOS_OBJ_548();
    return uVar2;
  default:
    puts("CDROM: unknown intr");
    printf("(%d)\n",(uint)in_stack_00000010);
    return 0;
  }
  if (unaff_s1 != 0) {
    DAT_8006035c = 5;
    puVar4 = &DAT_800a3240;
    puVar5 = (undefined4 *)&stack0x00000018;
    iVar6 = 7;
    do {
      uVar1 = *(undefined1 *)puVar5;
      puVar5 = (undefined4 *)((int)puVar5 + 1);
      iVar6 = iVar6 + -1;
      *puVar4 = uVar1;
      puVar4 = puVar4 + 1;
    } while (iVar6 != -1);
    uVar2 = BIOS_OBJ_548();
    return uVar2;
  }
  if (*(int *)(&DAT_80060144 + (uint)DAT_8006009d * 4) == 0) {
    DAT_8006035c = 2;
    puVar4 = &DAT_800a3240;
    puVar5 = (undefined4 *)&stack0x00000018;
    iVar6 = 7;
    do {
      uVar1 = *(undefined1 *)puVar5;
      puVar5 = (undefined4 *)((int)puVar5 + 1);
      iVar6 = iVar6 + -1;
      *puVar4 = uVar1;
      puVar4 = puVar4 + 1;
    } while (iVar6 != -1);
    uVar2 = BIOS_OBJ_548();
    return uVar2;
  }
  DAT_8006035c = 3;
  puVar4 = &DAT_800a3240;
  puVar5 = (undefined4 *)&stack0x00000018;
  iVar6 = 7;
  do {
    uVar1 = *(undefined1 *)puVar5;
    puVar5 = (undefined4 *)((int)puVar5 + 1);
    iVar6 = iVar6 + -1;
    *puVar4 = uVar1;
    puVar4 = puVar4 + 1;
  } while (iVar6 != -1);
  uVar2 = BIOS_OBJ_548();
  return uVar2;
}

