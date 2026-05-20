// addr: 0x80054984  name: FUN_80054984

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint FUN_80054984(int param_1,undefined1 param_2)

{
  byte bVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  word wVar5;
  
  wVar5 = 0x88;
  if (((int)(uint)**(byte **)(param_1 + 0x3c) >> 4 == 8) && (8 < *(byte *)(param_1 + 0x44))) {
    wVar5 = 0x22;
  }
  do {
  } while (((ushort)JOY_MCD_STAT & 2) == 0);
  FUN_800566b4(400);
  bVar1 = (byte)JOY_MCD_DATA;
  uVar2 = (uint)(byte)JOY_MCD_DATA;
  JOY_MCD_BAUD = wVar5;
  if ((*(char *)(param_1 + 0x44) == '\0') && ((int)uVar2 >> 4 == 8)) {
    JOY_MCD_BAUD = 0x22;
  }
  while( true ) {
    if ((_I_STAT & 0x80) != 0) {
      if ((*(char *)(param_1 + 0xe8) != '\b') && (DAT_800652a0 == 2)) {
        FUN_800566b4(0x3c);
        do {
          iVar4 = FUN_800566d4();
        } while (iVar4 == 0);
      }
      JOY_MCD_DATA._0_1_ = param_2;
      *(char *)(param_1 + 0x45) = *(char *)(param_1 + 0x45) + '\x01';
      if (*(char *)(param_1 + 0x44) != -1) {
        *(byte *)(*(int *)(param_1 + 0x3c) + (uint)*(byte *)(param_1 + 0x44)) = bVar1;
      }
      *(char *)(param_1 + 0x44) = *(char *)(param_1 + 0x44) + '\x01';
      return uVar2;
    }
    uVar3 = (uint)(ushort)TMR_SYSCLOCK_VAL;
    if (uVar3 < DAT_800a4f08) {
      if ((ushort)TMR_SYSCLOCK_MAX == 0) {
        uVar3 = uVar3 + 0x10000;
      }
      else {
        uVar3 = (ushort)TMR_SYSCLOCK_MAX + uVar3;
      }
    }
    if ((((ushort)TMR_SYSCLOCK_MODE & 0x200) != 0) && (DAT_800a4f0c <= uVar3 - DAT_800a4f08)) break;
    if (DAT_800a4f0c <= uVar3 - DAT_800a4f08 >> 3) {
      return 0xfffffffe;
    }
  }
  return 0xfffffffe;
}

