// addr: 0x8004ee10  name: RATAN_OBJ_13C

int RATAN_OBJ_13C(undefined4 param_1,undefined4 param_2,int param_3,int param_4)

{
  int in_v0;
  int iVar1;
  
  iVar1 = 0x400 - *(short *)(&DAT_800647b4 + in_v0);
  if (param_3 != 0) {
    iVar1 = 0x800 - iVar1;
  }
  if (param_4 != 0) {
    iVar1 = -iVar1;
  }
  return iVar1;
}

