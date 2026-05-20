// addr: 0x80045f84  name: SPU_OBJ_AB0

void SPU_OBJ_AB0(void)

{
  SPU_DELAY = SPU_DELAY & 0xf0ffffff | 0x22000000;
  return;
}

