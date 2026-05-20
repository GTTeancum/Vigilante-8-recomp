// addr: 0x80015098  name: FUN_80015098

undefined4 FUN_80015098(void)

{
  long lVar1;
  
  FUN_800116ec();
  FUN_80044fbc(&DAT_800a4f18,0x15b0e8U - iRam00000000 & 0xfffffff8);
  FUN_80015e8c();
  ResetGraph(0);
  SetGraphDebug(0);
  InitGeom();
  lVar1 = GetRCnt(0xf2000002);
  FUN_8001714c(lVar1);
  SetRCnt(0xf2000002,0xffff,0x1000);
  FUN_80053a24();
  lVar1 = OpenEvent(0xf2000002,2,0x1000,FUN_80014ff0);
  EnableEvent(lVar1);
  FUN_80053a34();
  FUN_80013cac();
  return 0;
}

