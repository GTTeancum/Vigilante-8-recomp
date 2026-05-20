// addr: 0x8003fea8  name: FUN_8003fea8

undefined4 * FUN_8003fea8(undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 *puVar2;
  
  if ((iRam00000010 == 0) && (iVar1 = FUN_8001db54(param_1,0), iVar1 != 0)) {
    puVar2 = (undefined4 *)FUN_8001d470(0x80);
    *puVar2 = 0xa0;
    puVar2[0xb] = 0x80;
    puVar2[9] = param_2;
    puVar2[0x19] = &LAB_8003ee88;
    FUN_800202f4(puVar2);
  }
  else {
    puVar2 = (undefined4 *)0x0;
  }
  return puVar2;
}

