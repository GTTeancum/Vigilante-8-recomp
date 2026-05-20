// addr: 0x8001de08  name: FUN_8001de08

void FUN_8001de08(uint *param_1)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  MATRIX *m2;
  MATRIX local_50;
  MATRIX local_30;
  
  if ((*param_1 & 2) == 0) {
    iVar1 = FUN_8001db54(param_1 + 9,param_1[0x15]);
    m2 = &local_50;
    if ((iVar1 != 0) &&
       (CompMatrixLV((MATRIX *)&DAT_8006f680,(MATRIX *)(param_1 + 4),m2), local_50.t[2] < 0x400000))
    {
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
      uVar2 = 0x40;
      if ((*param_1 & 0x2000) != 0) {
        uVar2 = param_1[0x12];
        if ((int)uVar2 < 0) {
          uVar2 = uVar2 + 0xffff;
        }
        uVar3 = param_1[0x14];
        if ((int)uVar3 < 0) {
          uVar3 = uVar3 + 0xffff;
        }
        uVar2 = (*(ushort *)
                  ((&DAT_800911a0)
                   [((uint)((int)uVar2 >> 0x10) >> 6) * 0x20 + ((uint)((int)uVar3 >> 0x10) >> 6)] +
                  ((int)uVar3 >> 0x10 & 0x3fU) * 2 + ((int)uVar2 >> 0x10 & 0x3fU) * 0x80) & 0xf800)
                >> 8;
      }
      SetBackColor(uVar2,uVar2,uVar2);
      if ((param_1[0x1b] == 0) || (local_50.t[2] <= (int)param_1[0x1b])) {
        if (param_1[0xc] != 0) {
          FUN_8001be5c(param_1[0xc],m2,uRam0000060c);
        }
        if (param_1[0xe] != 0) {
          FUN_8001dcc8(param_1[0xe],&local_50);
        }
      }
      else if (param_1[0x1a] != 0) {
        if ((*param_1 & 0x1010) == 0x1000) {
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
              FUN_80016e64(&local_50);
            }
          }
          else {
            local_50.m[0][0] = 0x1000;
            local_50.m[0][1] = 0;
            local_50.m[0][2] = 0;
            local_50.m[1][0] = 0;
            local_50.m[1][1] = 0x1000;
            local_50.m[1][2] = 0;
            local_50.m[2][0] = 0;
            local_50.m[2][1] = 0;
            local_50.m[2][2] = 0x1000;
          }
        }
        FUN_8001be5c(param_1[0x1a],m2,uRam0000060c);
      }
      if ((*param_1 & 8) != 0) {
        if ((*param_1 & 0x200) == 0) {
          FUN_8003e2fc(param_1);
        }
        FUN_8003e520(param_1[0x1c]);
      }
    }
  }
  return;
}

