// addr: 0x8010d1c8  name: FUN_8010d1c8

/* WARNING: Removing unreachable block (ram,0x8010d2a4) */

ulonglong FUN_8010d1c8(void)

{
  int iVar1;
  int iVar2;
  
  DAT_80113404 = 0;
  DAT_801133f8 = 1 - DAT_801133f8;
  DAT_801133fc = (&DAT_801133e8)[DAT_801133f8 * 4];
  DAT_801133fe = (&DAT_801133ea)[DAT_801133f8 * 4];
  iVar1 = -1;
  if (0 < (int)(&DAT_801133c0 + DAT_801133e4)) {
    for (iVar2 = 0x7fffff; iVar1 = DAT_801133f8, iVar2 != 0; iVar2 = iVar2 + -1) {
    }
  }
  return CONCAT44(DAT_80113402 + 0xf,iVar1) & 0xfffffff0ffffffff;
}

