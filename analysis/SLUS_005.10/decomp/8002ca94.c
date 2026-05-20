// addr: 0x8002ca94  name: FUN_8002ca94

void FUN_8002ca94(int param_1,uint param_2)

{
  byte bVar1;
  short sVar2;
  int iVar3;
  uint uVar4;
  
  bVar1 = *(byte *)(param_1 + 0xb3);
  uVar4 = (uint)bVar1;
  FUN_8002c99c();
  iVar3 = param_2 * 4 + param_1;
  FUN_80044c44(iVar3 + 0x110,iVar3 + 0x114,(2 - param_2) * 4);
  *(undefined4 *)(param_1 + 0x118) = 0;
  if ((int)uVar4 <= (int)param_2) {
    if (param_2 == 0) goto LAB_8002cb18;
    if (uVar4 != param_2) {
      return;
    }
    if (*(int *)(iVar3 + 0x110) != 0) goto LAB_8002cb18;
  }
  *(byte *)(param_1 + 0xb3) = bVar1 - 1;
LAB_8002cb18:
  if (uVar4 == param_2) {
    iVar3 = *(int *)(param_1 + (uint)*(byte *)(param_1 + 0xb3) * 4 + 0x110);
    if (iVar3 != 0) {
      sVar2 = 0x1e;
      if (0x1e < *(short *)(iVar3 + 6)) {
        sVar2 = *(short *)(iVar3 + 6);
      }
      *(short *)(iVar3 + 6) = sVar2;
    }
  }
  return;
}

