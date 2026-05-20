// addr: 0x801005e4  name: FUN_801005e4

void FUN_801005e4(int param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  
  if (param_2 != 1) {
    if (param_2 != 2) goto LAB_80100664;
    uVar2 = 1;
    if (*(short *)(param_1 + 10) == 1) {
      uVar2 = 2;
    }
    func_0x80044ac8(*(undefined4 *)(*(int *)(param_1 + 0x58) + 8),uVar2,param_1 + 0x48);
  }
  iVar1 = FUN_80017160/*0x80017160*/();
  FUN_80020890/*0x80020890*/(param_1,(iVar1 * 0x78 >> 0xf) + 0x3c);
LAB_80100664:
  func_0x800223dc(param_1,param_2,param_3);
  return;
}

