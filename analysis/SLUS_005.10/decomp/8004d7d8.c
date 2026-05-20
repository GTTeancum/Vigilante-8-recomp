// addr: 0x8004d7d8  name: FGO_04_OBJ_64

void FGO_04_OBJ_64(undefined4 param_1,int param_2)

{
  short sVar1;
  short sVar2;
  short sVar3;
  int in_t0;
  int in_t1;
  
  sVar1 = *(short *)(param_2 + 6);
  sVar2 = *(short *)(param_2 + 8);
  sVar3 = *(short *)(param_2 + 10);
  *(short *)(param_2 + 6) = (short)(in_t0 * sVar1 - in_t1 * *(short *)(param_2 + 0xc) >> 0xc);
  *(short *)(param_2 + 8) = (short)(in_t0 * sVar2 - in_t1 * *(short *)(param_2 + 0xe) >> 0xc);
  *(short *)(param_2 + 10) = (short)(in_t0 * sVar3 - in_t1 * *(short *)(param_2 + 0x10) >> 0xc);
  *(short *)(param_2 + 0xc) = (short)(in_t1 * sVar1 + in_t0 * *(short *)(param_2 + 0xc) >> 0xc);
  *(short *)(param_2 + 0xe) = (short)(in_t1 * sVar2 + in_t0 * *(short *)(param_2 + 0xe) >> 0xc);
  *(short *)(param_2 + 0x10) = (short)(in_t1 * sVar3 + in_t0 * *(short *)(param_2 + 0x10) >> 0xc);
  return;
}

