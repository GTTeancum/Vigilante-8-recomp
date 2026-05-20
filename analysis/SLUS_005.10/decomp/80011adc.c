// addr: 0x80011adc  name: FUN_80011adc

int * FUN_80011adc(void)

{
  int *piVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  int *piVar5;
  uint *puVar6;
  uint *puVar7;
  
  piVar1 = (int *)FUN_80015948();
  if (piVar1 != (int *)0x0) {
    puVar6 = (uint *)((int)piVar1 + *piVar1);
    if (*puVar6 != 0xffffffff) {
      uVar3 = *puVar6;
      do {
        puVar7 = puVar6 + 1;
        uVar4 = uVar3 & 3;
        piVar5 = (int *)((int)piVar1 + (uVar3 & 0xfffffffc));
        if (uVar4 == 1) {
          uVar3 = *puVar7;
          puVar7 = puVar6 + 2;
          *(short *)piVar5 = (short)((int)piVar1 + uVar3 + 0x8000 >> 0x10);
        }
        else if (uVar4 == 0) {
          iVar2 = *piVar5 + (int)piVar1;
LAB_80011bb0:
          *piVar5 = iVar2;
        }
        else if (uVar4 == 2) {
          *(short *)piVar5 = (short)*piVar5 + (short)piVar1;
        }
        else if (uVar4 == 3) {
          iVar2 = *piVar5 + ((uint)((int)piVar1 << 4) >> 6);
          goto LAB_80011bb0;
        }
        uVar3 = *puVar7;
        puVar6 = puVar7;
      } while (uVar3 != 0xffffffff);
    }
    FUN_80045134(piVar1,*piVar1);
  }
  return piVar1;
}

