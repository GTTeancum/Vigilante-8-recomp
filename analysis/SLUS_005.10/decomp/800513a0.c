// addr: 0x800513a0  name: SYS_OBJ_219C

uint SYS_OBJ_219C(uint param_1)

{
  GPU_REG1 = param_1 | 0x10000000;
  return GPU_REG0 & 0xffffff;
}

