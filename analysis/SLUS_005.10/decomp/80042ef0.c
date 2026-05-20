// addr: 0x80042ef0  name: FUN_80042ef0

void FUN_80042ef0(int param_1,undefined4 param_2,undefined4 *param_3,undefined4 param_4,
                 undefined4 param_5)

{
  undefined4 uVar1;
  int iVar2;
  
  uVar1 = FUN_80024d30(param_2,param_3,param_4,param_5);
  iVar2 = FUN_80042e78(param_1,uVar1);
  if (iVar2 == 0) {
    *(undefined4 *)(param_1 + 8) = *param_3;
    *(undefined4 *)(param_1 + 0xc) = param_3[2];
  }
  return;
}

