// addr: 0x80016678  name: FUN_80016678

void FUN_80016678(undefined4 param_1)

{
  int iVar1;
  
  FUN_80017e0c();
  FUN_80011834();
  iRam000006bc = FUN_80016024(param_1);
  iVar1 = VSyncCallback(FUN_80016364);
  *(int *)(iRam000006bc + 0x5dd0) = iVar1;
  DAT_8006f27c = 1;
  DAT_8006f220 = 1;
  return;
}

