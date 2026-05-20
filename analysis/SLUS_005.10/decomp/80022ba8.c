// addr: 0x80022ba8  name: FUN_80022ba8

void FUN_80022ba8(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  
  iVar1 = FUN_80011adc("Shell\\Load.dll");
  (**(code **)(iVar1 + 4))(param_1,param_2,param_3);
  FUN_80045088(iVar1);
  (*pcRam00000730)(puRam000006f8,1,0);
  if ((*puRam000006f8 & 0x80) != 0) {
    FUN_8001fe50(&DAT_80065a60);
  }
  return;
}

