// addr: 0x80100fa8  name: FUN_80100fa8

undefined4 FUN_80100fa8(int param_1,int *param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined2 local_40;
  undefined2 local_3e;
  undefined2 local_3c;
  int local_38;
  int local_34;
  int local_30;
  undefined1 auStack_28 [8];
  
  local_40 = *(undefined2 *)(param_1 + 0x12);
  local_3e = *(undefined2 *)(param_1 + 0x18);
  local_3c = *(undefined2 *)(param_1 + 0x1e);
  iVar1 = func_0x80016810(param_3,&local_40);
  if (iVar1 < 0) {
    local_38 = *param_2 - *(int *)(param_1 + 0x48);
    local_34 = param_2[1] - *(int *)(param_1 + 0x4c);
    local_30 = param_2[2] - *(int *)(param_1 + 0x50);
    FUN_80016a20/*0x80016a20*/(&local_38);
    uVar2 = FUN_80016b08/*0x80016b08*/(&local_38,auStack_28);
    func_0x80016810(uVar2,&local_40);
  }
  return 0;
}

