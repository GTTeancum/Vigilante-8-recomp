// addr: 0x80100d1c  name: FUN_80100d1c

void FUN_80100d1c(int param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  int iVar2;
  undefined4 local_res4 [3];
  
  local_res4[0] = param_2;
  iVar1 = func_0x800224ec(local_res4);
  if (4 < param_3) {
    func_0x800224ec(local_res4);
  }
  iVar2 = iVar1;
  if ((*(char *)(param_1 + 4) == '\x05') && (-1 < *(short *)(param_1 + 6))) {
    iVar2 = iVar1 * (DAT_8006531a + 2);
    iVar1 = iVar1 * (DAT_8006531a + 2);
    if (iVar2 < 0) {
      iVar2 = iVar2 + 3;
    }
    iVar2 = iVar2 >> 2;
    if (iVar1 < 0) {
      iVar1 = iVar1 + 3;
    }
    iVar1 = iVar1 >> 2;
  }
  *(short *)(param_1 + 0xc) = (short)iVar2;
  *(short *)(param_1 + 0xe) = (short)iVar1;
  for (iVar1 = *(int *)(param_1 + 0x38); iVar1 != 0; iVar1 = *(int *)(iVar1 + 0x34)) {
    if (*(short *)(iVar1 + 0xc) == 0) {
      *(short *)(iVar1 + 0xc) = (short)iVar2;
    }
  }
  return;
}

