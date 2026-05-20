// addr: 0x8004bcac  name: C_011_OBJ_258

void C_011_OBJ_258(void)

{
  uint *puVar1;
  undefined4 uVar2;
  uint uVar3;
  short *psVar4;
  uint in_stack_00000028;
  
  psVar4 = DAT_800a32d8;
  do {
  } while ((DMA_CDROM_CHCR & 0x1000000) != 0);
  uVar3 = (int)DAT_800a32d8 + 0x1fU & 3;
  puVar1 = (uint *)(((int)DAT_800a32d8 + 0x1fU) - uVar3);
  *puVar1 = *puVar1 & -1 << (uVar3 + 1) * 8 | in_stack_00000028 >> (3 - uVar3) * 8;
  uVar3 = (uint)(psVar4 + 0xe) & 3;
  puVar1 = (uint *)((int)(psVar4 + 0xe) - uVar3);
  *puVar1 = *puVar1 & 0xffffffffU >> (4 - uVar3) * 8 | in_stack_00000028 << uVar3 * 8;
  CDROM_DELAY = 0x20843;
  COMMON_DELAY = 0x1325;
  if ((DAT_800a32c0 == 1) && (DAT_800a329c != 0)) {
    if (DAT_800a329c != (ushort)DAT_800a32d8[4]) {
      *DAT_800a32d8 = 0;
      if (DAT_800a32b8 == 0) {
        return;
      }
      DAT_800a32a8 = DAT_800a32a8 + 1;
      C_011_OBJ_90C();
      return;
    }
    DAT_800a32c0 = 0;
  }
  if ((*DAT_800a32d8 != 0x160) || (((ushort)DAT_800a32d8[1] >> 10 & 0x1f) != DAT_800a32a0)) {
    if (DAT_800a32b8 != 0) {
      DAT_800a32a8 = 0;
      C_011_OBJ_3AC();
      return;
    }
    DAT_8006047c = 5;
    *DAT_800a32d8 = 0;
    C_011_OBJ_90C();
    return;
  }
  if (((int)DAT_800a328c != (uint)(ushort)DAT_800a32d8[2]) ||
     ((DAT_800a3288 != 0 && (DAT_800a3288 != (ushort)DAT_800a32d8[4])))) {
    DAT_800a3288 = 0;
    DAT_800a328c = 0;
    init_ring_status(DAT_800a32b0,DAT_800a32ac - DAT_800a32b0);
    DAT_800a32ac = DAT_800a32b0;
    *DAT_800a32d8 = 0;
    if (DAT_800a32b8 != 0) {
      DAT_800a32a8 = DAT_800a32a8 + 1;
    }
    DAT_8006047c = 6;
    C_011_OBJ_90C();
    return;
  }
  if (DAT_800a32d8[2] == 0) {
    DAT_800a3288 = (uint)(ushort)DAT_800a32d8[4];
    DAT_800a328c = 0;
    if ((DAT_800a32bc != 0) && (DAT_800a32bc <= DAT_800a3288)) {
      DAT_800a3288 = 0;
      DAT_800a328c = 0;
      init_ring_status(DAT_800a32b0,DAT_800a32ac - DAT_800a32b0);
      DAT_800a32ac = DAT_800a32b0;
      *DAT_800a32d8 = 0;
      DAT_800a32c0 = 1;
      if (DAT_800a32d4 != (code *)0x0) {
        (*DAT_800a32d4)();
      }
      if (DAT_800a32b8 != 0) {
        DAT_800a32a8 = DAT_800a32a8 + 1;
      }
      DAT_8006047c = 7;
      C_011_OBJ_90C();
      return;
    }
    if ((DAT_800a32cc - DAT_800a32ac) - 1U < (uint)(ushort)DAT_800a32d8[3]) {
      if (DAT_800a32bc == 0) {
        *DAT_800a32d8 = 1;
        DAT_800a32c0 = 1;
        if (DAT_800a32d4 != (code *)0x0) {
          (*DAT_800a32d4)();
        }
        if (DAT_800a32b8 != 0) {
          DAT_800a32a8 = DAT_800a32a8 + 1;
        }
        DAT_8006047c = 8;
        C_011_OBJ_90C();
        return;
      }
      if (*DAT_800a32c8 != 0) {
        *DAT_800a32d8 = 0;
        if (DAT_800a32b8 != 0) {
          DAT_800a32a8 = DAT_800a32a8 + 1;
        }
        DAT_8006047c = 9;
        C_011_OBJ_90C();
        return;
      }
      *DAT_800a32d8 = 1;
      uVar3 = 0;
      DAT_800a32ac = 0;
      psVar4 = DAT_800a32c8;
      do {
        uVar2 = *(undefined4 *)DAT_800a32d8;
        DAT_800a32d8 = DAT_800a32d8 + 2;
        uVar3 = uVar3 + 1;
        *(undefined4 *)psVar4 = uVar2;
        psVar4 = psVar4 + 2;
      } while (uVar3 < 8);
      DAT_800a32d8 = DAT_800a32c8;
    }
    DAT_800a32b0 = DAT_800a32ac;
  }
  DAT_8006047c = 10;
  DAT_800a328c = DAT_800a328c + 1;
  DAT_800a32c4 = DAT_800a32c8 + DAT_800a32cc * 0x10 + DAT_800a32ac * 0x3f0;
  if (DAT_800a3290 != 0) {
    CDROM_DELAY = 0x20943;
    COMMON_DELAY = 0x1323;
    C_011_OBJ_778();
    return;
  }
  CDROM_DELAY = 0x21020843;
  if ((ushort)DAT_800a32d8[3] - 1 != (uint)(ushort)DAT_800a32d8[2]) {
    if (DAT_800a32b8 == 0) {
      C_011_OBJ_948(3,DAT_800a32c4,0,0x1f8);
      COMMON_DELAY = 0x1325;
      *DAT_800a32d8 = 3;
      DAT_800a32ac = DAT_800a32ac + 1;
      if ((DAT_800a32b8 != 0) && (DAT_800a32a4 != 0)) {
        data_ready_callback();
      }
      return;
    }
    C_011_OBJ_91C(DAT_800a32c4,DAT_800a32b8 + DAT_800a32a8 * 0x800 + 0x20,0x1f8,0);
    DAT_800a32a8 = DAT_800a32a8 + 1;
    C_011_OBJ_8B0();
    return;
  }
  DAT_800a32a4 = 1;
  if (DAT_800a32b8 != 0) {
    C_011_OBJ_91C(DAT_800a32c4,DAT_800a32b8 + DAT_800a32a8 * 0x800 + 0x20,0x1f8,1);
    DAT_800a32a8 = DAT_800a32a8 + 1;
    C_011_OBJ_814();
    return;
  }
  C_011_OBJ_948(3,DAT_800a32c4,0,0x1f8);
  DAT_800a328c = 0;
  DAT_800a3288 = 0;
  DAT_800a32a0 = DAT_800a3298;
  C_011_OBJ_8B0();
  return;
}

