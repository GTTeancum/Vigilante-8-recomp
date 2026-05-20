// addr: 0x80020000  name: FUN_80020000

undefined4 FUN_80020000(void)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_8001ff58();
  uVar2 = 0;
  if (iVar1 != 0) {
    uVar2 = *(undefined4 *)(iVar1 + 8);
  }
  return uVar2;
}

