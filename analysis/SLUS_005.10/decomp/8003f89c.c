// addr: 0x8003f89c  name: FUN_8003f89c

void FUN_8003f89c(uint *param_1,int *param_2,ushort param_3,uint param_4)

{
  bool bVar1;
  undefined2 uVar2;
  undefined2 uVar3;
  uint uVar4;
  int iVar5;
  ushort *puVar6;
  
  bVar1 = false;
  puVar6 = (ushort *)(*param_2 + (uint)param_3 * 0x1c + 0x1c);
  if ((*param_1 & 4) != 0) {
    FUN_800207f8(param_1);
  }
  FUN_8001bddc(param_1[0xc]);
  FUN_800204dc(param_1[0xe]);
  if (param_1[0x1a] != 0) {
    FUN_8001bddc();
    param_1[0x1a] = 0;
    param_1[0x1b] = 0;
  }
  FUN_80043408(param_1 + 4,puVar6 + 2,param_1 + 0x12);
  param_1[9] = param_1[0x12];
  param_1[10] = param_1[0x13];
  param_1[0xb] = param_1[0x14];
  if (*puVar6 < 0xfffe) {
    uVar4 = FUN_8001b49c(param_2,*puVar6 & 0xff);
    param_1[0xc] = uVar4;
  }
  else {
    param_1[0xc] = 0;
  }
  param_1[0x18] = 0;
  *(ushort *)((int)param_1 + 10) = param_3;
  param_1[0xe] = param_4;
  if (param_4 != 0) {
    *(uint **)(param_4 + 0x3c) = param_1;
  }
  if ((short)puVar6[1] < 0) {
    param_1[0x17] = 0;
  }
  else {
    param_1[0x17] = *(uint *)((short)puVar6[1] * 4 + *(int *)(*param_2 + 0xc));
  }
  iVar5 = FUN_8001ec48(param_1);
  uVar3 = uRam000006cc;
  if (iVar5 == 0) {
    *param_1 = *param_1 | 0x20;
    uVar3 = uRam000006cc;
  }
  for (; uVar2 = uRam000006cc, param_4 != 0; param_4 = *(uint *)(param_4 + 0x34)) {
    if (*(int *)(param_4 + 0x60) != 0) {
      *(undefined2 *)(param_4 + 0x46) = uRam000006cc;
      uRam000006cc = uVar3;
      bVar1 = true;
      uVar3 = uRam000006cc;
    }
    uRam000006cc = uVar3;
    uVar3 = uRam000006cc;
    uRam000006cc = uVar2;
  }
  uRam000006cc = uVar3;
  if (bVar1) {
    FUN_800207c4(param_1);
  }
  *param_1 = *param_1 & 0xffff7fff;
  FUN_8001dc1c();
  return;
}

