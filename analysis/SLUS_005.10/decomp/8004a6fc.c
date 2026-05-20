// addr: 0x8004a6fc  name: CD_init

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 CD_init(void)

{
  int iVar1;
  undefined4 uVar2;
  
  puts("CD_init:");
  printf("addr=%08x\n",&PTR_DAT_80060360);
  DAT_8006009d = 0;
  DAT_8006009c = 0;
  DAT_80060080 = 0;
  DAT_8006007c = 0;
  DAT_80060090 = 0;
  _DAT_8006008c = 0;
  ResetCallback();
  InterruptCallback(2,BIOS_OBJ_1688);
  if ((CDROM_REG3 & 7) != 0) {
    do {
    } while( true );
  }
  DAT_8006035e = 0;
  DAT_8006035d = 0;
  DAT_8006035c = 2;
  CDROM_REG0 = 0;
  CDROM_REG3 = 0;
  COMMON_DELAY = 0x1325;
  CD_cw(1,0,0,0);
  if ((_DAT_8006008c & 0x10) != 0) {
    CD_cw(1,0,0,0);
  }
  iVar1 = CD_cw(10,0,0,0);
  if (((iVar1 == 0) && (iVar1 = CD_cw(0xc,0,0,0), iVar1 == 0)) && (iVar1 = CD_sync(0,0), iVar1 == 2)
     ) {
    uVar2 = BIOS_OBJ_1318();
    return uVar2;
  }
  return 0xffffffff;
}

