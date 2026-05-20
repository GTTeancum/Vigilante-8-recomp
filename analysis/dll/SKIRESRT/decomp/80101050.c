// addr: 0x80101050  name: FUN_80101050

undefined4 FUN_80101050(uint *param_1,int param_2)

{
  ushort uVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  uint *puVar5;
  uint uVar6;
  uint uVar7;
  
  if ((param_2 == 0) || (iVar2 = -0x7ffa0000, param_2 != 2)) {
    uVar3 = 0x10000;
    if ((*param_1 & 1) != 0) {
      uVar3 = param_1[0x20];
      if ((int)uVar3 < 0) {
        uVar3 = uVar3 + 0x7f;
      }
      uVar4 = param_1[0x21];
      param_1[9] = param_1[9] + ((int)uVar3 >> 7);
      if ((int)uVar4 < 0) {
        uVar4 = uVar4 + 0x7f;
      }
      uVar3 = param_1[0x22];
      param_1[10] = param_1[10] + ((int)uVar4 >> 7);
      if ((int)uVar3 < 0) {
        uVar3 = uVar3 + 0x7f;
      }
      uVar3 = (int)uVar3 >> 7;
      param_1[0xb] = param_1[0xb] + uVar3;
    }
    puVar5 = (uint *)param_1[0x1e];
    uVar1 = *(ushort *)((int)puVar5 + 0x46);
    uVar4 = puVar5[5];
    uVar6 = puVar5[6];
    uVar7 = puVar5[7];
    param_1[4] = puVar5[4];
    param_1[5] = uVar4;
    param_1[6] = uVar6;
    param_1[7] = uVar7;
    uVar4 = puVar5[9];
    uVar6 = puVar5[10];
    uVar7 = puVar5[0xb];
    param_1[8] = puVar5[8];
    param_1[9] = uVar4;
    param_1[10] = uVar6;
    param_1[0xb] = uVar7;
    uVar4 = puVar5[10];
    uVar6 = puVar5[0xb];
    param_1[0x12] = puVar5[9];
    param_1[0x13] = uVar4;
    param_1[0x14] = uVar6;
    param_1[10] = param_1[10] + (uVar3 | 0x5000);
    if (((*(short *)((int)puVar5 + 10) == 0x2c) && ((short)uVar1 < 0x6001)) &&
       (*(ushort *)((int)puVar5 + 0x46) < 0xe001)) {
      if ((*(uint *)(~(int)*(short *)((int)param_1 + 6) * 0x18 + -0x7ff9a3d0) & 0x1000000) == 0) {
        return 0;
      }
      if (*(ushort *)((int)puVar5 + 0x46) < 0x1001) {
        return 0;
      }
      if ((short)uVar1 < -0x6fff) {
        return 0;
      }
    }
    puVar5[0x20] = 0;
    *puVar5 = *puVar5 | 0x20;
    FUN_80020890/*0x80020890*/(puVar5,0x78);
    func_0x80031294(param_1);
    iVar2 = -0x1000023;
    puVar5 = param_1 + 0x20;
    param_1[0x20] = 0;
    *param_1 = *param_1 & 0xfeffffdd | 8;
    param_1[0x21] = 0x5f5;
    if ((int)((uint)uVar1 << 0x10) < 0) {
      puVar5 = (uint *)0x1;
    }
    puVar5[2] = 0x47800;
  }
  iVar2 = *(int *)(iVar2 + 0x59fc);
  *param_1 = *param_1 | 2;
  *(undefined2 *)(iVar2 + 0xa2) = 0x3c;
  return 0;
}

