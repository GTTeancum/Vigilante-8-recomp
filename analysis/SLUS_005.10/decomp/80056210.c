// addr: 0x80056210  name: PADPORTD_OBJ_63C

undefined * PADPORTD_OBJ_63C(uint param_1)

{
  undefined *puVar1;
  
  puVar1 = &DAT_800a4d28;
  if ((param_1 & 0xf0) != 0) {
    puVar1 = &DAT_800a4e18;
  }
  return puVar1;
}

