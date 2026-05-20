// addr: 0x80102334  name: FUN_80102334

void FUN_80102334(void)

{
  int *piVar1;
  int iVar2;
  
  iVar2 = 0;
  piVar1 = (int *)&DAT_800737a0;
  do {
    if (*piVar1 != 0) {
      FUN_8001aa38/*0x8001aa38*/();
    }
    iVar2 = iVar2 + 1;
    piVar1 = piVar1 + 1;
  } while (iVar2 < 0xe);
  return;
}

