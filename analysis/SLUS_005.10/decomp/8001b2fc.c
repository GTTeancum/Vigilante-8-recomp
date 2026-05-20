// addr: 0x8001b2fc  name: FUN_8001b2fc

void FUN_8001b2fc(undefined4 param_1,int param_2,int param_3)

{
  undefined2 uVar1;
  uint uVar2;
  uint *puVar3;
  uint in_v0;
  uint uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  
  uVar2 = param_2 + 0x13U & 3;
  uVar4 = param_2 + 0x10U & 3;
  uVar4 = (*(int *)((param_2 + 0x13U) - uVar2) << (3 - uVar2) * 8 |
          in_v0 & 0xffffffffU >> (uVar2 + 1) * 8) & -1 << (4 - uVar4) * 8 |
          *(uint *)((param_2 + 0x10U) - uVar4) >> uVar4 * 8;
  uVar1 = *(undefined2 *)(param_2 + 0x14);
  uVar2 = param_3 + 0x43U & 3;
  puVar3 = (uint *)((param_3 + 0x43U) - uVar2);
  *puVar3 = *puVar3 & -1 << (uVar2 + 1) * 8 | uVar4 >> (3 - uVar2) * 8;
  uVar2 = param_3 + 0x40U & 3;
  puVar3 = (uint *)((param_3 + 0x40U) - uVar2);
  *puVar3 = *puVar3 & 0xffffffffU >> (4 - uVar2) * 8 | uVar4 << uVar2 * 8;
  *(undefined2 *)(param_3 + 0x44) = uVar1;
  uVar5 = *(undefined4 *)(param_2 + 8);
  uVar6 = *(undefined4 *)(param_2 + 0xc);
  *(undefined4 *)(param_3 + 0x48) = *(undefined4 *)(param_2 + 4);
  *(undefined4 *)(param_3 + 0x4c) = uVar5;
  *(undefined4 *)(param_3 + 0x50) = uVar6;
  FUN_8001d708(param_3);
  FUN_8001d4f0(param_1,param_3);
  return;
}

