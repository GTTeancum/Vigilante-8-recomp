// addr: 0x800547ac  name: FUN_800547ac

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint FUN_800547ac(int param_1,int param_2)

{
  byte bVar1;
  byte bVar2;
  byte bVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  
  bVar2 = (byte)JOY_MCD_DATA;
  bVar1 = (byte)param_2;
  if (param_2 < 0) {
    uVar5 = (uint)(byte)JOY_MCD_DATA;
    *(undefined1 *)(param_1 + 0x44) = 0xff;
    *(undefined1 *)(param_1 + 0x45) = 1;
    **(byte **)(param_1 + 0x40) = ~bVar1;
    do {
    } while (((ushort)JOY_MCD_STAT & 1) == 0);
    do {
      iVar4 = FUN_800566d4();
      bVar3 = ~bVar1;
    } while (iVar4 == 0);
  }
  else {
    JOY_MCD_BAUD = 0x88;
    if (((int)(uint)**(byte **)(param_1 + 0x3c) >> 4 == 8) &&
       (JOY_MCD_BAUD = 0x88, 8 < *(byte *)(param_1 + 0x44))) {
      JOY_MCD_BAUD = 0x22;
    }
    DAT_800a4f08 = (uint)(ushort)TMR_SYSCLOCK_VAL;
    DAT_800a4f0c = 0x1ae;
    do {
    } while (((ushort)JOY_MCD_STAT & 2) == 0);
    uVar6 = (uint)(byte)JOY_MCD_DATA;
    do {
      if ((_I_STAT & 0x80) != 0) {
        JOY_MCD_DATA._0_1_ = bVar1;
        *(char *)(param_1 + 0x45) = *(char *)(param_1 + 0x45) + '\x01';
        *(byte *)(*(int *)(param_1 + 0x3c) + (uint)*(byte *)(param_1 + 0x44)) = bVar2;
        *(char *)(param_1 + 0x44) = *(char *)(param_1 + 0x44) + '\x01';
        return uVar6;
      }
      iVar4 = FUN_800566d4();
      uVar5 = 0xffffffec;
      bVar3 = (byte)JOY_MCD_DATA;
    } while (iVar4 == 0);
  }
  JOY_MCD_DATA._0_1_ = bVar3;
  return uVar5;
}

