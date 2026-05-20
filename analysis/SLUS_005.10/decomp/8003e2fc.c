// addr: 0x8003e2fc  name: FUN_8003e2fc

void FUN_8003e2fc(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 *puVar3;
  short local_70;
  short local_6e;
  short local_6c;
  int local_68 [4];
  MATRIX local_58;
  MATRIX MStack_38;
  
  puVar3 = *(undefined4 **)(param_1 + 0x70);
  uVar1 = FUN_80025400(*(undefined4 *)(param_1 + 0x24),*(undefined4 *)(param_1 + 0x2c));
  puVar3[6] = *(undefined4 *)(param_1 + 0x24);
  puVar3[8] = *(undefined4 *)(param_1 + 0x2c);
  if (*(int *)(param_1 + 0x74) != 0) {
    iVar2 = FUN_8001f51c(*(int *)(param_1 + 0x74),uVar1,param_1 + 0x24,&local_70);
    if ((iVar2 != 0) ||
       ((*(int *)(param_1 + 0x78) != 0 &&
        (iVar2 = FUN_8001f51c(*(int *)(param_1 + 0x78),uVar1,param_1 + 0x24,&local_70), iVar2 != 0))
       )) {
      puVar3[7] = iVar2;
      goto LAB_8003e3b0;
    }
  }
  puVar3[7] = uVar1;
  FUN_80025800(*(undefined4 *)(param_1 + 0x24),*(undefined4 *)(param_1 + 0x2c),&local_70);
LAB_8003e3b0:
  if ((*(ushort *)*puVar3 & 8) == 0) {
    local_58.m[0][0] = 0x1000;
    local_58.m[2][2] = 0x1000;
    local_58.m[2][1] = 0;
    local_58.m[2][0] = 0;
    local_58.m[1][1] = 0;
    local_58.m[0][2] = 0;
    local_58.m[0][1] = 0;
    if (local_6e == 0) {
      local_58.m[1][0] = local_70 * -0x10;
    }
    else {
      local_58.m[1][0] = (short)((local_70 * -0x1000) / (int)local_6e);
    }
    if (local_6e == 0) {
      local_58.m[1][2] = local_6c * -0x10;
    }
    else {
      local_58.m[1][2] = (short)((local_6c * -0x1000) / (int)local_6e);
    }
    local_68[0] = puVar3[9];
    if (*(short *)(param_1 + 0x18) < 1) {
      local_68[0] = -local_68[0];
    }
    local_68[1] = 0;
    local_68[2] = puVar3[10];
    FUN_80043754(param_1 + 0x10,local_68,&MStack_38);
    MulMatrix0(&local_58,&MStack_38,(MATRIX *)(puVar3 + 1));
  }
  else {
    *(undefined2 *)(puVar3 + 1) = 0x1000;
    *(undefined2 *)(puVar3 + 5) = 0x1000;
    *(undefined2 *)((int)puVar3 + 0x12) = 0;
    *(undefined2 *)(puVar3 + 4) = 0;
    *(undefined2 *)(puVar3 + 3) = 0;
    *(undefined2 *)(puVar3 + 2) = 0;
    *(undefined2 *)((int)puVar3 + 6) = 0;
    *(short *)((int)puVar3 + 10) = (short)((local_70 * -0x1000) / (int)local_6e);
    *(short *)((int)puVar3 + 0xe) = (short)((local_6c * -0x1000) / (int)local_6e);
  }
  return;
}

