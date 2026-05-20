// addr: 0x8010072c  name: FUN_8010072c

undefined4 FUN_8010072c(uint *param_1)

{
  int iVar1;
  uint uVar2;
  int local_28 [4];
  short local_18 [2];
  undefined2 local_14;
  
  if (param_1[0x29] != 0) {
    if ((*param_1 & 0x10000) != 0) {
      uVar2 = 0;
      if (0 < (int)(param_1[0x2a] - 0xe)) {
        uVar2 = param_1[0x2a] - 0xe;
      }
      param_1[0x2a] = uVar2;
    }
    uVar2 = param_1[0x2c];
    if ((int)uVar2 < 0) {
      uVar2 = uVar2 + 0xffff;
    }
    func_0x80042390(param_1[0x29],(int)uVar2 >> 0x10,param_1 + 9,local_28);
    local_28[1] = 0;
    if (param_1[0x2b] == 0) {
      local_28[0] = -local_28[0];
      local_28[2] = -local_28[2];
    }
    VectorNormalS/*0x8004c800*/(local_28,local_18);
    *(undefined2 *)(param_1 + 8) = local_14;
    *(undefined2 *)(param_1 + 4) = local_14;
    *(short *)(param_1 + 5) = local_18[0];
    *(short *)(param_1 + 7) = -local_18[0];
    iVar1 = SquareRoot0/*0x8004c6e4*/(local_28[0] * local_28[0] + local_28[2] * local_28[2]);
    if (param_1[0x2b] != 0) {
      iVar1 = 1;
    }
    uVar2 = param_1[0x2c] + (int)(param_1[0x2a] * -0x10000) / iVar1;
    param_1[0x2c] = uVar2;
    if (uVar2 < 0x10000001) {
      return 0;
    }
    uVar2 = func_0x80042698(param_1[0x29],param_1 + 0x2b);
    param_1[0x29] = uVar2;
    if (uVar2 != 0) {
      param_1[0x2c] = -(uint)(param_1[0x2b] == 0) & 0x10000000;
    }
    iVar1 = (int)(short)param_1[5] * param_1[0x2a];
    *param_1 = *param_1 & 0xfffffeff;
    if (iVar1 < 0) {
      iVar1 = iVar1 + 0x1f;
    }
    param_1[0x20] = iVar1 >> 5;
    param_1[0x21] = 0;
    iVar1 = (int)(short)param_1[8] * param_1[0x2a];
    if (iVar1 < 0) {
      iVar1 = iVar1 + 0x1f;
    }
    param_1[0x22] = iVar1 >> 5;
    FUN_80020890/*0x80020890*/(param_1,0x78);
  }
  FUN_8001787c/*0x8001787c*/(param_1,param_1[0x17] + 4);
  return 0;
}

