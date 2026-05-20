// addr: 0x80015948  name: FUN_80015948

undefined4 FUN_80015948(void)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_800157d4();
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else {
    uVar2 = FUN_800116f4(*(int *)(iVar1 + 0x10) + 0x7ffU & 0xfffff800);
    uVar2 = FUN_800154f4(uVar2,*(undefined4 *)(iVar1 + 0xc),*(int *)(iVar1 + 0x10) + 0x7ffU >> 0xb);
    uVar2 = FUN_80045134(uVar2,*(undefined4 *)(iVar1 + 0x10));
  }
  return uVar2;
}

