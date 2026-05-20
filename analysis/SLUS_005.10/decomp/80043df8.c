// addr: 0x80043df8  name: FUN_80043df8

undefined4 FUN_80043df8(undefined4 param_1,undefined1 param_2)

{
  int iVar1;
  undefined4 uVar2;
  CdlLOC aCStack_20 [2];
  u_char local_18;
  undefined1 local_17;
  
  iVar1 = FUN_800157d4();
  uRam000008f4 = 0;
  iRam000008ec = 0;
  iRam000008e8 = 0;
  uVar2 = 0;
  if (iVar1 != 0) {
    CdIntToPos(*(int *)(iVar1 + 0xc),aCStack_20);
    uRam000008df = 0xff;
    iRam000008e0 = *(int *)(iVar1 + 0xc) + (*(uint *)(iVar1 + 0x10) >> 0xb) + -2;
    do {
      local_18 = '\x01';
      local_17 = param_2;
      while (iVar1 = CdControl('\r',&local_18,(u_char *)0x0), iVar1 == 0) {
        iRam000008e8 = iRam000008e8 + 1;
      }
      while (iVar1 = CdControl('\x02',&aCStack_20[0].minute,(u_char *)0x0), iVar1 == 0) {
        iRam000008ec = iRam000008ec + 1;
      }
      iVar1 = CdRead2(0xe8);
    } while (iVar1 == 0);
    CdReadyCallback(FUN_80043d94);
    uVar2 = 1;
  }
  return uVar2;
}

