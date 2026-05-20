// addr: 0x80021394  name: FUN_80021394

void FUN_80021394(uint param_1)

{
  int *piVar1;
  int iVar2;
  int *piVar3;
  uint *puVar4;
  
  if (puRam000007c4 != &DAT_80065ac0) {
    do {
      piVar1 = piRam000007bc;
      if (param_1 < (uint)piRam000007bc[3]) {
        return;
      }
      puVar4 = (uint *)piRam000007bc[2];
      *puVar4 = *puVar4 & 0xfffffffe;
      piVar3 = (int *)piVar1[1];
      iVar2 = *piVar1;
      *(int **)(iVar2 + 4) = piVar3;
      *piVar3 = iVar2;
      piVar3 = piRam00000774;
      piRam00000774 = piVar1;
      *piVar3 = (int)piVar1;
      piVar1[1] = (int)piVar3;
      *piVar1 = (int)&DAT_80065a74;
      piVar1[2] = 0;
      if ((code *)puVar4[0x19] != (code *)0x0) {
        (*(code *)puVar4[0x19])(puVar4,2,0);
      }
    } while (puRam000007c4 != &DAT_80065ac0);
  }
  return;
}

