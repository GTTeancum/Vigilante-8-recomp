// addr: 0x8004f204  name: ResetGraph

int ResetGraph(int mode)

{
  int iVar1;
  uint uVar2;
  
  uVar2 = mode & 7;
  if (uVar2 != 3) {
    if (3 < uVar2) {
      if (uVar2 != 5) {
        iVar1 = SYS_OBJ_110();
        return iVar1;
      }
      goto SYS_OBJ_68;
    }
    if (uVar2 != 0) {
      iVar1 = SYS_OBJ_110();
      return iVar1;
    }
  }
  printf("ResetGraph:jtb=%08x,env=%08x\n",&PTR_s__Id__sys_c_v_1_140_1998_01_12_07_80064fdc,
         &DAT_80065024);
SYS_OBJ_68:
  SYS_OBJ_2FE4(&DAT_80065024,0,0x80);
  ResetCallback();
  GPU_cw(0x64fdc);
  DAT_80065024 = SYS_OBJ_2700(mode);
  DAT_80065025 = 1;
  DAT_80065028 = *(undefined2 *)(&DAT_800650a4 + (uint)DAT_80065024 * 4);
  DAT_8006502a = *(undefined2 *)(&DAT_800650b0 + (uint)DAT_80065024 * 4);
  SYS_OBJ_2FE4(&DAT_80065034,0xffffffff,0x5c);
  SYS_OBJ_2FE4(&DAT_80065090,0xffffffff,0x14);
  iVar1 = SYS_OBJ_160();
  return iVar1;
}

