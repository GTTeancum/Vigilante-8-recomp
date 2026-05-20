// addr: 0x80043a74  name: FUN_80043a74

void FUN_80043a74(void)

{
  uint uVar1;
  int iVar2;
  
  do {
    uVar1 = CdGetToc((CdlLOC *)&DAT_800a3090);
    bRam000008f8 = (byte)uVar1;
  } while ((uVar1 & 0xff) == 0);
  iVar2 = CdPosToInt((CdlLOC *)&DAT_800a3090);
  CdIntToPos(iVar2 + -0x96,(CdlLOC *)(&UNK_800a3094 + (uint)bRam000008f8 * 4));
  bRam000008f8 = bRam000008f8 - (2 < uRam000005ac);
  return;
}

