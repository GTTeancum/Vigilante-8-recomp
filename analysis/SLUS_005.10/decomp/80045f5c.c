// addr: 0x80045f5c  name: SPU_OBJ_A88

void SPU_OBJ_A88(void)

{
  SPU_DELAY = SPU_DELAY & 0xf0ffffff | 0x20000000;
  return;
}

