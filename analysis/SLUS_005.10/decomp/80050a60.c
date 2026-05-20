// addr: 0x80050a60  name: SYS_OBJ_185C

uint SYS_OBJ_185C(uint param_1,uint param_2)

{
  return (param_2 & 0x7ff) << 0xb | param_1 & 0x7ff | 0xe5000000;
}

