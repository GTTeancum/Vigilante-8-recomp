// addr: 0x8002169c  name: FUN_8002169c

void FUN_8002169c(void)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int *piVar4;
  int iVar5;
  uint *puVar6;
  
  piVar4 = (int *)*piRam00000714;
  piVar2 = piRam00000714;
  do {
    piVar1 = piVar4;
    if (piVar1 == (int *)0x0) {
      return;
    }
    puVar6 = (uint *)piVar2[2];
    if ((*puVar6 & 0x20) == 0) {
      puVar6[0x1e] = 0;
      puVar6[0x1d] = 0;
      piVar4 = (int *)*piVar1;
      piVar2 = piVar1;
      while (piVar3 = piVar4, piVar3 != (int *)0x0) {
        if ((((**(uint **)((int)piVar2 + 8) & 0x20) == 0) &&
            ((**(uint **)((int)piVar2 + 8) & *puVar6 & 0x200) == 0)) &&
           (iVar5 = FUN_8001edb4(puVar6), iVar5 != 0)) {
          return;
        }
        piVar2 = piVar3;
        piVar4 = (int *)*piVar3;
      }
      if ((*puVar6 & 0x100) == 0) {
        FUN_80020f14(uRam000006fc,puVar6);
      }
    }
    piVar4 = (int *)*piVar1;
    piVar2 = piVar1;
  } while( true );
}

