// addr: 0x8001a4f8  name: FUN_8001a4f8

void FUN_8001a4f8(int param_1,int param_2)

{
  u_long *p;
  
  if (*(int *)(param_1 + 0x10) != 0) {
    if (param_2 == 0) {
      *(uint *)(param_1 + 0x30) = param_1 + 0x48U & 0xffffff | 0x5000000;
    }
    else {
      *(undefined4 *)(param_1 + 0x30) = 0;
    }
  }
  p = (u_long *)(param_1 + 0x18);
  if ((param_2 == 0) && (p = (u_long *)(param_1 + 0x30), *(int *)(param_1 + 0x10) == 0)) {
    p = (u_long *)(param_1 + 0x48);
  }
  DrawOTag(p);
  FUN_8004d524(*(undefined4 *)(param_1 + 8),*(undefined4 *)(param_1 + 0xc));
  return;
}

