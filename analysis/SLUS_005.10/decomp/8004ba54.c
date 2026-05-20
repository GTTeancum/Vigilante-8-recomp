// addr: 0x8004ba54  name: StCdInterrupt

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void StCdInterrupt(void)

{
  uint *puVar1;
  int iVar2;
  undefined4 uVar3;
  byte *pbVar4;
  uint uVar5;
  short *psVar6;
  uint local_18 [2];
  byte local_10 [8];
  
  if (DAT_800a32a4 != 1) {
    if ((DAT_800a3290 != 0) && ((DMA_MDEC_OUT_CHCR & 0x1000000) != 0)) {
      DAT_800a3294 = 1;
      if (DAT_800a32b8 != 0) {
        DAT_800a32a8 = DAT_800a32a8 + 1;
      }
      DAT_8006047c = 1;
      C_011_OBJ_90C();
      return;
    }
    iVar2 = CdReady(1,local_10);
    if (iVar2 != 5) {
      if ((local_10[0] & 4) != 0) {
        DAT_8006047c = 3;
        C_011_OBJ_90C();
        return;
      }
      DAT_800a32d8 = DAT_800a32c8 + DAT_800a32ac * 0x10;
      if (*DAT_800a32d8 != 0) {
        if (DAT_800a32b8 != 0) {
          DAT_800a32a8 = DAT_800a32a8 + 1;
        }
        DAT_8006047c = 4;
        C_011_OBJ_90C();
        return;
      }
      CDROM_REG0 = 0;
      CDROM_REG3 = 0x80;
      CDROM_DELAY = 0x20943;
      COMMON_DELAY = 0x1323;
      uVar5 = 0;
      if (DAT_800a3270 == 0) {
        do {
          pbVar4 = (byte *)((int)local_18 + uVar5);
          uVar5 = uVar5 + 1;
          *pbVar4 = CDROM_REG2;
        } while (uVar5 < 4);
        uVar5 = 0;
        do {
          uVar5 = uVar5 + 1;
        } while (uVar5 < 8);
      }
      if (DAT_800a32b8 != 0) {
        C_011_OBJ_91C(DAT_800a32d8,DAT_800a32b8 + DAT_800a32a8 * 0x800,8,0);
        C_011_OBJ_258();
        return;
      }
      C_011_OBJ_948(3,DAT_800a32d8,0,8,0x11000000,0,0);
      psVar6 = DAT_800a32d8;
      do {
      } while ((DMA_CDROM_CHCR & 0x1000000) != 0);
      uVar5 = (int)DAT_800a32d8 + 0x1fU & 3;
      puVar1 = (uint *)(((int)DAT_800a32d8 + 0x1fU) - uVar5);
      *puVar1 = *puVar1 & -1 << (uVar5 + 1) * 8 | local_18[0] >> (3 - uVar5) * 8;
      uVar5 = (uint)(psVar6 + 0xe) & 3;
      puVar1 = (uint *)((int)(psVar6 + 0xe) - uVar5);
      *puVar1 = *puVar1 & 0xffffffffU >> (4 - uVar5) * 8 | local_18[0] << uVar5 * 8;
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
          uVar5 = 0;
          DAT_800a32ac = 0;
          psVar6 = DAT_800a32c8;
          do {
            uVar3 = *(undefined4 *)DAT_800a32d8;
            DAT_800a32d8 = DAT_800a32d8 + 2;
            uVar5 = uVar5 + 1;
            *(undefined4 *)psVar6 = uVar3;
            psVar6 = psVar6 + 2;
          } while (uVar5 < 8);
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
      if ((ushort)DAT_800a32d8[3] - 1 == (uint)(ushort)DAT_800a32d8[2]) {
        DAT_800a32a4 = 1;
        if (DAT_800a32b8 != 0) {
          C_011_OBJ_91C(DAT_800a32c4,DAT_800a32b8 + DAT_800a32a8 * 0x800 + 0x20,0x1f8,1);
          DAT_800a32a8 = DAT_800a32a8 + 1;
          C_011_OBJ_814();
          return;
        }
        C_011_OBJ_948(3,DAT_800a32c4,0,0x1f8,0x11400100,1,0);
        DAT_800a328c = 0;
        DAT_800a3288 = 0;
        DAT_800a32a0 = DAT_800a3298;
        C_011_OBJ_8B0();
        return;
      }
      if (DAT_800a32b8 != 0) {
        C_011_OBJ_91C(DAT_800a32c4,DAT_800a32b8 + DAT_800a32a8 * 0x800 + 0x20,0x1f8,0);
        DAT_800a32a8 = DAT_800a32a8 + 1;
        C_011_OBJ_8B0();
        return;
      }
      C_011_OBJ_948(3,DAT_800a32c4,0,0x1f8,0x11400100,0,0);
      COMMON_DELAY = 0x1325;
      *DAT_800a32d8 = 3;
      DAT_800a32ac = DAT_800a32ac + 1;
      if ((DAT_800a32b8 != 0) && (DAT_800a32a4 != 0)) {
        data_ready_callback();
      }
    }
  }
  return;
}

