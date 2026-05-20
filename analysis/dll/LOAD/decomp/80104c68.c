// addr: 0x80104c68  name: FUN_80104c68

void FUN_80104c68(char *param_1,undefined4 param_2,undefined4 param_3)

{
  char cVar1;
  int local_18 [2];
  int local_10;
  
  FUN_80043224/*0x80043224*/(param_3,local_18);
  if (local_18[0] < 0) {
    local_18[0] = local_18[0] + 0xffff;
  }
  if (local_10 < 0) {
    local_10 = local_10 + 0xffff;
  }
  cVar1 = (byte)((ushort)*(undefined2 *)
                          (*(int *)(((uint)(local_10 >> 0x10) >> 6) * 4 +
                                    ((uint)(local_18[0] >> 0x10) >> 6) * 0x80 + -0x7ff6ee60) +
                          (local_10 >> 0x10 & 0x3fU) * 2 + (local_18[0] >> 0x10 & 0x3fU) * 0x80) >>
                0xb) << 2;
  param_1[2] = cVar1;
  param_1[1] = cVar1;
  *param_1 = cVar1;
  return;
}

