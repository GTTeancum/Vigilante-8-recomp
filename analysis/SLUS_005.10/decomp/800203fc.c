// addr: 0x800203fc  name: FUN_800203fc

uint * FUN_800203fc(uint *param_1)

{
  if ((code *)param_1[0x19] != (code *)0x0) {
    (*(code *)param_1[0x19])(param_1,4,0);
  }
  if ((*param_1 & 0x80) != 0) {
    FUN_8001fe8c(&DAT_80065a60,param_1);
  }
  if ((*param_1 & 4) != 0) {
    FUN_8001fe8c(&DAT_80065a80,param_1);
  }
  if ((*param_1 & 1) != 0) {
    FUN_8001fe8c(&DAT_80065ac0,param_1);
  }
  return param_1;
}

