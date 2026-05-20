// addr: 0x8004b040  name: CDREAD_OBJ_32C

undefined4 CDREAD_OBJ_32C(int param_1)

{
  uint uVar1;
  undefined4 uVar2;
  CdlLOC *pCVar3;
  int iVar4;
  uint uVar5;
  u_char local_18 [8];
  
  CdSyncCallback((CdlCB)0x0);
  CdReadyCallback((CdlCB)0x0);
  if ((DAT_800603c8 & 1) != 0) {
    CdDataCallback((func *)0x0);
  }
  uVar1 = CdStatus();
  if ((uVar1 & 0x10) != 0) {
    uVar1 = VSync(-1);
    if ((uVar1 & 0x3f) == 0) {
      puts("CdRead: Shell open...\n");
    }
    CdControlF('\x01',(u_char *)0x0);
    DAT_800603b4 = VSync(-1);
    DAT_800603ac = 0xffffffff;
    uVar2 = CDREAD_OBJ_504();
    return uVar2;
  }
  if (param_1 != 0) {
    puts("CdRead: retry...\n");
    CdControl('\t',(u_char *)0x0,(u_char *)0x0);
    pCVar3 = CdLastPos();
    iVar4 = CdControl('\x02',&pCVar3->minute,(u_char *)0x0);
    if (iVar4 == 0) {
      DAT_800603ac = 0xffffffff;
      uVar2 = CDREAD_OBJ_504();
      return uVar2;
    }
  }
  CdFlush();
  local_18[0] = (u_char)DAT_800603a4;
  uVar5 = DAT_800603a4 & 0xff;
  uVar1 = CdMode();
  if (((uVar5 != uVar1) || (param_1 != 0)) &&
     (iVar4 = CdControl('\x0e',local_18,(u_char *)0x0), iVar4 == 0)) {
    DAT_800603ac = 0xffffffff;
    uVar2 = CDREAD_OBJ_504();
    return uVar2;
  }
  pCVar3 = CdLastPos();
  DAT_800603b8 = CdPosToInt(pCVar3);
  CdReadyCallback(CDREAD_OBJ_0);
  if ((DAT_800603c8 & 1) != 0) {
    CdDataCallback(CDREAD_OBJ_260);
  }
  DAT_800603a0 = DAT_8006039c;
  CdControlF('\x06',(u_char *)0x0);
  DAT_800603ac = DAT_80060398;
  DAT_800603b0 = VSync(-1);
  return DAT_800603ac;
}

