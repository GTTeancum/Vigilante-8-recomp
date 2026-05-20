// addr: 0x800154f4  name: FUN_800154f4

u_long * FUN_800154f4(u_long *param_1,int param_2,int param_3)

{
  CdlLOC aCStack_18 [2];
  
  CdIntToPos(param_2,aCStack_18);
  CdControl('\x15',&aCStack_18[0].minute,(u_char *)0x0);
  CdRead(param_3,param_1,0x80);
  CdReadSync(0,(u_char *)0x0);
  return param_1;
}

