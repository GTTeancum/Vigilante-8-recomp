// addr: 0x80050a44  name: SYS_OBJ_1840

uint SYS_OBJ_1840(uint param_1,uint param_2)

{
  return (param_2 & 0x3ff) << 10 | param_1 & 0x3ff | 0xe4000000;
}

