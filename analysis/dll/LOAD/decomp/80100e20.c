// addr: 0x80100e20  name: FUN_80100e20

void FUN_80100e20(int param_1,undefined4 param_2)

{
  undefined2 uVar1;
  undefined4 uVar2;
  undefined4 local_res4 [3];
  
  local_res4[0] = param_2;
  uVar2 = func_0x800224ec(local_res4);
  *(undefined4 *)(param_1 + 0x80) = uVar2;
  uVar2 = func_0x800224ec(local_res4);
  *(undefined4 *)(param_1 + 0x84) = uVar2;
  uVar2 = func_0x800224ec(local_res4);
  *(undefined4 *)(param_1 + 0x88) = uVar2;
  uVar1 = func_0x800224b4(local_res4);
  *(undefined2 *)(param_1 + 0x8c) = uVar1;
  uVar1 = func_0x800224b4(local_res4);
  *(undefined2 *)(param_1 + 0x8e) = uVar1;
  uVar1 = func_0x800224b4(local_res4);
  *(undefined2 *)(param_1 + 0x90) = uVar1;
  return;
}

