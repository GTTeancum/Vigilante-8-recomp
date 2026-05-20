// addr: 0x8002036c  name: FUN_8002036c

undefined4 FUN_8002036c(uint *param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  FUN_8001d708();
  FUN_8001dc1c(param_1);
  if ((code *)param_1[0x19] == (code *)0x0) {
    iVar1 = 0;
  }
  else {
    iVar1 = (*(code *)param_1[0x19])(param_1,1,0);
  }
  uVar2 = 0;
  if (-1 < iVar1) {
    if (((*param_1 & 8) != 0) && (param_1[0x1c] == 0)) {
      FUN_8003e730(param_1);
    }
    uVar2 = FUN_800202f4(param_1);
  }
  return uVar2;
}

