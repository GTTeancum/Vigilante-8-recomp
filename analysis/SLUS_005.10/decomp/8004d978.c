// addr: 0x8004d978  name: FGO_05_OBJ_64

void FGO_05_OBJ_64(undefined4 param_1,short *param_2)

{
  short sVar1;
  short sVar2;
  short sVar3;
  int in_t0;
  int in_t1;
  
  sVar1 = *param_2;
  sVar2 = param_2[1];
  sVar3 = param_2[2];
  *param_2 = (short)(in_t0 * sVar1 - in_t1 * param_2[6] >> 0xc);
  param_2[1] = (short)(in_t0 * sVar2 - in_t1 * param_2[7] >> 0xc);
  param_2[2] = (short)(in_t0 * sVar3 - in_t1 * param_2[8] >> 0xc);
  param_2[6] = (short)(in_t1 * sVar1 + in_t0 * param_2[6] >> 0xc);
  param_2[7] = (short)(in_t1 * sVar2 + in_t0 * param_2[7] >> 0xc);
  param_2[8] = (short)(in_t1 * sVar3 + in_t0 * param_2[8] >> 0xc);
  return;
}

