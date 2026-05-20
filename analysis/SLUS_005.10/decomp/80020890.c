// addr: 0x80020890  name: FUN_80020890

void FUN_80020890(uint *param_1,int param_2)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  int *piVar4;
  int *piVar5;
  undefined4 *puVar6;
  int iVar7;
  
  if ((*param_1 & 1) != 0) {
    FUN_8001fe8c(&DAT_80065ac0,param_1);
  }
  piVar4 = piRam0000076c;
  iVar3 = iRam0000000c;
  iVar7 = *piRam0000076c;
  *(undefined **)(iVar7 + 4) = &DAT_80065a70;
  piVar2 = piRam0000076c + 2;
  piRam0000076c = (int *)iVar7;
  *piVar2 = (int)param_1;
  *param_1 = *param_1 | 1;
  piVar4[3] = param_2 + iVar3;
  piVar5 = (int *)*piRam000007bc;
  piVar2 = piRam000007bc;
  while ((piVar1 = piVar5, piVar1 != (int *)0x0 && ((uint)piVar2[3] < (uint)(param_2 + iVar3)))) {
    piVar5 = (int *)*piVar1;
    piVar2 = piVar1;
  }
  puVar6 = (undefined4 *)piVar2[1];
  *puVar6 = piVar4;
  piVar2[1] = (int)piVar4;
  *piVar4 = (int)piVar2;
  piVar4[1] = (int)puVar6;
  return;
}

