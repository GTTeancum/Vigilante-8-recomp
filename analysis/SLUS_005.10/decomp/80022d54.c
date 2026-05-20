// addr: 0x80022d54  name: FUN_80022d54

void FUN_80022d54(uint *param_1)

{
  short sVar1;
  int iVar2;
  undefined4 uVar3;
  uint uVar4;
  
  sVar1 = FUN_800244c4(param_1[9],param_1[0xb]);
  if (sVar1 != 0) {
    FUN_80042ef0(param_1 + 0x30,param_1 + 9,param_1[0x39] + 0x24,0x22740,0);
    *(undefined1 *)(param_1 + 2) = 3;
    *param_1 = *param_1 & 0xffffffdf;
  }
  uVar4 = param_1[0x43];
  *(undefined2 *)(param_1 + 0x29) = 0;
  *(undefined2 *)((int)param_1 + 0xa6) = 0x3c;
  uVar3 = 0xb;
  if (*(char *)(uVar4 + 8) == '\0') {
    if (*(code **)(uVar4 + 100) == (code *)0x0) {
      iVar2 = 0;
    }
    else {
      iVar2 = (**(code **)(uVar4 + 100))(uVar4,0xc,param_1);
    }
    uVar3 = 0xb;
    if (iVar2 == 0) {
      uVar3 = 4;
    }
  }
  if (*(code **)(uVar4 + 100) != (code *)0x0) {
    (**(code **)(uVar4 + 100))(uVar4,uVar3,param_1);
  }
  return;
}

