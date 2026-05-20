// addr: 0x8001d898  name: FUN_8001d898

void FUN_8001d898(MATRIX *param_1)

{
  undefined1 local_50 [8];
  short local_48 [2];
  SVECTOR local_44;
  long local_3c;
  long local_38;
  long local_34;
  short local_30 [2];
  short local_2c [3];
  short sStack_26;
  short local_24;
  short sStack_22;
  short local_20;
  undefined2 uStack_1e;
  long local_1c;
  long local_18;
  long local_14;
  
  FUN_80044efc(local_30,0,0x20);
  local_30[0] = sRam000006d4;
  sStack_22 = -sRam000006d4;
  sStack_26 = (short)(-iRam000006d8 / 2);
  local_50._6_2_ = sStack_22;
  local_50._4_2_ = sStack_26;
  local_20 = (short)(-iRam000006dc / 2);
  local_50._0_2_ = sRam000006d4;
  local_50._2_2_ = local_30[1];
  local_48[0] = local_2c[2];
  local_48[1] = sStack_26;
  local_44.vx = local_24;
  local_44.vy = sStack_22;
  local_44.vz = local_20;
  local_44.pad = uStack_1e;
  local_3c = local_1c;
  local_38 = local_18;
  local_34 = local_14;
  local_2c._0_4_ = local_50._4_4_;
  VectorNormalSS((SVECTOR *)local_50,(SVECTOR *)local_50);
  VectorNormalSS((SVECTOR *)(local_50 + 6),(SVECTOR *)(local_50 + 6));
  VectorNormalSS((SVECTOR *)(local_50 + 0xc),(SVECTOR *)(local_50 + 0xc));
  MulMatrix0((MATRIX *)local_50,param_1,(MATRIX *)&DAT_8006f780);
  return;
}

