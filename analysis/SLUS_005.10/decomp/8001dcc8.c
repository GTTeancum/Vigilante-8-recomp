// addr: 0x8001dcc8  name: FUN_8001dcc8

void FUN_8001dcc8(uint *param_1,MATRIX *param_2)

{
  MATRIX *m2;
  MATRIX local_50;
  MATRIX local_30;
  
  do {
    m2 = &local_50;
    CompMatrixLV(param_2,(MATRIX *)(param_1 + 4),m2);
    if ((*param_1 & 0x10) != 0) {
      if ((*param_1 & 0x400) == 0) {
        if (*(short *)((int)param_1 + 0x22) == 0) {
          local_50.m[0][0] = (short)DAT_8006f660;
          local_50.m[0][1] = DAT_8006f660._2_2_;
          local_50.m[0][2] = (short)DAT_8006f664;
          local_50.m[1][0] = DAT_8006f664._2_2_;
          local_50.m[1][1] = (short)DAT_8006f668;
          local_50.m[1][2] = DAT_8006f668._2_2_;
          local_50.m[2][0] = (short)DAT_8006f66c;
          local_50.m[2][1] = DAT_8006f66c._2_2_;
          local_50.m[2][2] = DAT_8006f670;
        }
        else {
          FUN_80016e64(m2);
        }
      }
      else {
        local_30.m[0]._0_4_ = param_1[4];
        local_30.m._4_4_ = param_1[5];
        local_30.m[1]._2_4_ = param_1[6];
        local_30.m[2]._0_4_ = param_1[7];
        local_30.m[2][2] = (short)param_1[8];
        local_30.t[0] = local_50.t[0];
        local_30.t[1] = local_50.t[1];
        local_30.t[2] = local_50.t[2];
        m2 = &local_30;
      }
    }
    if (param_1[0xc] != 0) {
      FUN_8001be5c(param_1[0xc],m2,uRam0000060c);
    }
    if (param_1[0xe] != 0) {
      FUN_8001dcc8(param_1[0xe],&local_50);
    }
    param_1 = (uint *)param_1[0xd];
  } while (param_1 != (uint *)0x0);
  return;
}

