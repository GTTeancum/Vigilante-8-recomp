// addr: 0x800512e0  name: SYS_OBJ_20DC

void SYS_OBJ_20DC(uint param_1)

{
  GPU_REG1 = param_1;
  (&DAT_800a3340)[param_1 >> 0x18] = (char)param_1;
  return;
}

