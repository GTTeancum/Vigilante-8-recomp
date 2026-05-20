// addr: 0x80011834  name: FUN_80011834

void FUN_80011834(void)

{
  int *piVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  
  piVar4 = (int *)*piRamffff9a48;
  iVar2 = piVar4[1];
  while (iVar2 != 0) {
    piVar4 = (int *)*piVar4;
    iVar2 = piVar4[1];
  }
  iVar2 = ((int *)*piVar4)[1];
  piVar3 = (int *)*piVar4;
  piRamffff9a48 = piVar4;
  while (piVar1 = piVar4, piVar4 = piVar3, iVar2 != 0) {
    piVar3 = (int *)*piVar4;
    iVar2 = piVar3[1];
    piRamffff9a48 = piVar1;
  }
  return;
}

