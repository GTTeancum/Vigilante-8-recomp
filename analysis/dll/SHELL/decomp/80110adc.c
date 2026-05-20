// addr: 0x80110adc  name: FUN_80110adc

void FUN_80110adc(int *param_1,undefined4 param_2,undefined4 param_3)

{
  code *pcVar1;
  int in_zero;
  int iVar2;
  int *piVar3;
  int *piVar4;
  
  if (*param_1 == 0) {
    *param_1 = 1;
  }
  pcVar1 = DAT_80113490;
  piVar4 = *(int **)(in_zero + 0x150);
  piVar3 = piVar4 + (*(uint *)(in_zero + 0x154) / 0x50) * 0x14;
  for (; piVar4 < piVar3; piVar4 = piVar4 + 0x14) {
    if ((*piVar4 != 0) && (iVar2 = strcmp/*0x80052484*/(*piVar4,&DAT_80113498), iVar2 == 0)) {
      piVar4[0xd] = (int)pcVar1;
    }
  }
  (*DAT_80113490)(param_1,param_2,param_3);
  return;
}

