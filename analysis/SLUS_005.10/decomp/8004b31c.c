// addr: 0x8004b31c  name: CDREAD_OBJ_608

bool CDREAD_OBJ_608(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 in_v0;
  uint uVar1;
  int iVar2;
  
  DAT_800603a4 = DAT_800603a4 | 0x20;
  DAT_80060398 = param_4;
  DAT_8006039c = param_2;
  DAT_800603a8 = in_v0;
  DAT_800603bc = CdSyncCallback((CdlCB)0x0);
  DAT_800603c0 = CdReadyCallback((CdlCB)0x0);
  if ((DAT_800603c8 & 1) != 0) {
    DAT_800603c4 = CdDataCallback((func *)0x0);
  }
  DAT_800603b4 = VSync(-1);
  uVar1 = CdStatus();
  if ((uVar1 & 0xe0) != 0) {
    CdControlB('\t',(u_char *)0x0,(u_char *)0x0);
  }
  iVar2 = CDREAD_OBJ_32C(0);
  return 0 < iVar2;
}

