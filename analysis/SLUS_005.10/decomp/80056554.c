// addr: 0x80056554  name: PADSEQD_OBJ_320

void PADSEQD_OBJ_320(void)

{
  char cVar1;
  int unaff_s0;
  
  if (*(code **)(unaff_s0 + 0x18) != (code *)0x0) {
    (**(code **)(unaff_s0 + 0x18))();
    PADSEQD_OBJ_348();
    return;
  }
  cVar1 = _padRecvAtLoadInfo();
  *(char *)(unaff_s0 + 0x46) = *(char *)(unaff_s0 + 0x46) + cVar1;
  return;
}

