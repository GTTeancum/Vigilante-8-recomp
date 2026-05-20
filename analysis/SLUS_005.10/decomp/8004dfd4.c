// addr: 0x8004dfd4  name: DIVF3A_OBJ_320

void DIVF3A_OBJ_320(uint *param_1,int param_2)

{
  int in_t0;
  int in_t1;
  int in_t2;
  uint uVar1;
  uint uVar2;
  uint uVar3;
  
  uVar3 = *(uint *)(param_2 + 0x10);
  uVar1 = *(uint *)(in_t1 + 0x10);
  uVar2 = *(uint *)(in_t2 + 0x10);
  param_1[2] = *(uint *)(in_t0 + 0x10);
  param_1[3] = uVar1;
  param_1[4] = uVar2;
  param_1[1] = uVar3;
  uVar1 = **(uint **)(param_2 + 0x14);
  **(uint **)(param_2 + 0x14) = (uint)param_1 & 0xffffff;
  *param_1 = uVar1 | 0x4000000;
  return;
}

