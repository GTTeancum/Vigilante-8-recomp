// addr: 0x800159b4  name: FUN_800159b4

undefined4 FUN_800159b4(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_800157d4();
  if (iVar1 == 0) {
    uVar2 = FUN_80015368(param_1);
  }
  else {
    FUN_8001570c(*(undefined4 *)(iVar1 + 0xc));
    uRam000006ac = 0;
    uVar2 = 1;
  }
  return uVar2;
}

