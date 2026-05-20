// addr: 0x800509ac  name: SYS_OBJ_17A8

uint SYS_OBJ_17A8(uint param_1,uint param_2)

{
  return (param_2 & 0x3ff) << 10 | param_1 & 0x3ff | 0xe3000000;
}

