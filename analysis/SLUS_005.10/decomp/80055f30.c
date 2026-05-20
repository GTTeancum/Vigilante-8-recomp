// addr: 0x80055f30  name: PADPORTD_OBJ_35C

undefined1 PADPORTD_OBJ_35C(int param_1)

{
  undefined1 uVar1;
  int in_v1;
  
  uVar1 = 0;
  if (in_v1 < (int)(uint)*(byte *)(param_1 + 0x35)) {
    uVar1 = *(undefined1 *)(*(int *)(param_1 + 0x2c) + in_v1);
  }
  return uVar1;
}

