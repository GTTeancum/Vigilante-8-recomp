// addr: 0x800165cc  name: FUN_800165cc

void FUN_800165cc(int param_1)

{
  int iVar1;
  
  if (iRam000006bc != 0) {
    if (param_1 == 0) {
      iVar1 = *(int *)(iRam000006bc + 0x5dcc);
      while (iVar1 == 0) {
        iVar1 = *(int *)(iRam000006bc + 0x5dcc);
      }
    }
    VSyncCallback(*(f **)(iRam000006bc + 0x5dd0));
    PutDrawEnv((DRAWENV *)(&DAT_8006f208 + (1 - iRam00000004) * 0x5c));
    FUN_80045088(iRam000006bc);
    iRam000006bc = 0;
    DAT_8006f27c = 0;
    DAT_8006f220 = 0;
  }
  return;
}

