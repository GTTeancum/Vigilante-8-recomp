// addr: 0x80043ce0  name: FUN_80043ce0

void FUN_80043ce0(int param_1)

{
  u_char local_10 [8];
  
  local_10[0] = '\x05';
  uRam000008f0 = (undefined1)param_1;
  if (uRam000005ac < 3) {
    param_1 = param_1 + 1;
  }
  else {
    param_1 = param_1 + 2;
  }
  DAT_80065be0 = *(undefined4 *)(&DAT_800a3090 + param_1 * 4);
  iRam000008e0 = CdPosToInt((CdlLOC *)(&UNK_800a3094 + param_1 * 4));
  iRam000008e0 = iRam000008e0 + -0x96;
  CdControl('\x0e',local_10,(u_char *)0x0);
  CdControl('\x03',(u_char *)&DAT_80065be0,(u_char *)0x0);
  CdReadyCallback(FUN_80043c34);
  return;
}

