// addr: 0x80020120  name: FUN_80020120

int FUN_80020120(int *param_1,uint param_2)

{
  int *piVar1;
  int *piVar2;
  uint uVar3;
  int iVar4;
  
  iVar4 = 0;
  piVar1 = (int *)*param_1;
  for (piVar2 = (int *)*(int *)*param_1; piVar2 != (int *)0x0; piVar2 = (int *)*piVar2) {
    if (((0x1f < *(short *)(piVar1[2] + 6)) && (uVar3 = *(uint *)piVar1[2], (uVar3 & param_2) != 0))
       && ((uVar3 & 0x8002) == 0)) {
      iVar4 = iVar4 + 1;
    }
    piVar1 = piVar2;
  }
  return iVar4;
}

