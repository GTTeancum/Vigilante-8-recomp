// addr: 0x80044efc  name: FUN_80044efc

uint * FUN_80044efc(uint *param_1,uint param_2,int param_3)

{
  uint uVar1;
  uint *puVar2;
  uint *puVar3;
  undefined1 *puVar4;
  uint uVar5;
  
  puVar3 = param_1;
  while( true ) {
    if (param_3 == 0) {
      return puVar3;
    }
    uVar5 = param_2 & 0xff;
    if (((uint)param_1 & 3) == 0) break;
    *(char *)param_1 = (char)param_2;
    param_3 = param_3 + -1;
    param_1 = (uint *)((int)param_1 + 1);
    param_2 = uVar5;
  }
  uVar5 = uVar5 | uVar5 << 8;
  uVar5 = uVar5 | uVar5 << 0x10;
  while (-1 < param_3 + -4) {
    *param_1 = uVar5;
    param_1 = param_1 + 1;
    param_3 = param_3 + -4;
  }
  puVar4 = (undefined1 *)((int)param_1 + param_3 + -1);
  if (param_3 + -1 < 0) {
    return puVar3;
  }
  uVar1 = (uint)puVar4 & 3;
  puVar2 = (uint *)(puVar4 + -uVar1);
  *puVar2 = *puVar2 & -1 << (uVar1 + 1) * 8 | uVar5 >> (3 - uVar1) * 8;
  return puVar3;
}

