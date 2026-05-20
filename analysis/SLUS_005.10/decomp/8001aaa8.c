// addr: 0x8001aaa8  name: FUN_8001aaa8

int * FUN_8001aaa8(int *param_1,ushort *param_2,undefined4 param_3,uint param_4)

{
  ushort uVar1;
  uint uVar2;
  uint *puVar3;
  int *piVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  
  piVar4 = (int *)FUN_8001178c(param_3,1);
  iVar5 = 0;
  if (-1 < (short)*param_2) {
    iVar5 = (uint)((*param_2 & 0x800) != 0) << 4;
  }
  *piVar4 = iVar5;
  uVar2 = (int)param_2 + 0x13U & 3;
  uVar6 = (uint)(param_2 + 8) & 3;
  uVar6 = (*(int *)(((int)param_2 + 0x13U) - uVar2) << (3 - uVar2) * 8 |
          param_4 & 0xffffffffU >> (uVar2 + 1) * 8) & -1 << (4 - uVar6) * 8 |
          *(uint *)((int)(param_2 + 8) - uVar6) >> uVar6 * 8;
  uVar1 = param_2[10];
  uVar2 = (int)piVar4 + 0x43U & 3;
  puVar3 = (uint *)(((int)piVar4 + 0x43U) - uVar2);
  *puVar3 = *puVar3 & -1 << (uVar2 + 1) * 8 | uVar6 >> (3 - uVar2) * 8;
  uVar2 = (uint)(piVar4 + 0x10) & 3;
  puVar3 = (uint *)((int)(piVar4 + 0x10) - uVar2);
  *puVar3 = *puVar3 & 0xffffffffU >> (4 - uVar2) * 8 | uVar6 << uVar2 * 8;
  *(ushort *)(piVar4 + 0x11) = uVar1;
  iVar5 = *(int *)(param_2 + 4);
  iVar7 = *(int *)(param_2 + 6);
  piVar4[0x12] = *(int *)(param_2 + 2);
  piVar4[0x13] = iVar5;
  piVar4[0x14] = iVar7;
  piVar4[0x16] = (int)param_1;
  if (-1 < (short)*param_2) {
    iVar5 = FUN_8001b49c(param_1,*param_2 & 0x7ff);
    piVar4[0xc] = iVar5;
  }
  if (-1 < (short)param_2[1]) {
    piVar4[0x17] = *(int *)((short)param_2[1] * 4 + *(int *)(*param_1 + 0xc));
  }
  FUN_8001d708(piVar4);
  return piVar4;
}

