// addr: 0x80015bf0  name: FUN_80015bf0

void FUN_80015bf0(uint param_1,int param_2)

{
  int iVar1;
  
  if (param_2 != 0) {
    param_1 = param_1 + uRam000006ac;
  }
  iVar1 = (param_1 >> 0xb) - ((int)uRam000006ac >> 0xb);
  while (iVar1 = iVar1 + -1, iVar1 != -1) {
    uRam0000069c = FUN_800156d4();
  }
  uRam000006ac = param_1;
  return;
}

