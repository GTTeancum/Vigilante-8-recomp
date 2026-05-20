// addr: 0x8004e7d0  name: DIVG3A_OBJ_3AC

void DIVG3A_OBJ_3AC(uint *param_1,int param_2)

{
  int in_t0;
  int in_t1;
  int in_t2;
  uint uVar1;
  uint uVar2;
  uint uVar3;
  
  *(undefined1 *)(in_t0 + 0xf) = *(undefined1 *)(param_2 + 0x13);
  uVar1 = *(uint *)(in_t1 + 0x10);
  uVar2 = *(uint *)(in_t2 + 0x10);
  param_1[2] = *(uint *)(in_t0 + 0x10);
  param_1[4] = uVar1;
  param_1[6] = uVar2;
  uVar3 = **(uint **)(param_2 + 0x14);
  **(uint **)(param_2 + 0x14) = (uint)param_1 & 0xffffff;
  uVar1 = *(uint *)(in_t1 + 0xc);
  uVar2 = *(uint *)(in_t2 + 0xc);
  param_1[1] = *(uint *)(in_t0 + 0xc);
  param_1[3] = uVar1;
  param_1[5] = uVar2;
  *param_1 = uVar3 | 0x6000000;
  return;
}

