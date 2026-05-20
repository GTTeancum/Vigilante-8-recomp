// addr: 0x800128d4  name: FUN_800128d4

void FUN_800128d4(void)

{
  int *piVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  int *piVar5;
  int *piVar6;
  int iVar7;
  
  piVar1 = (int *)FUN_80015f80("Quest.bin");
  piVar6 = piVar1 + 1;
  iVar7 = 0;
  piVar5 = piVar1;
  piRam00000608 = piVar1;
  if (0 < *piVar1) {
    do {
      piVar5 = piVar5 + 2;
      iVar2 = *piVar5;
      *piVar5 = iVar2 + (int)piVar1;
      iVar4 = 0;
      if (0 < *piVar6) {
        piVar3 = (int *)(iVar2 + (int)piVar1 + 8);
        do {
          piVar3[1] = piVar3[1] + (int)piVar1;
          *piVar3 = *piVar3 + (int)piVar1;
          iVar4 = iVar4 + 1;
          piVar3 = piVar3 + 4;
        } while (iVar4 < *piVar6);
      }
      iVar7 = iVar7 + 1;
      piVar6 = piVar6 + 2;
    } while (iVar7 < *piVar1);
  }
  return;
}

