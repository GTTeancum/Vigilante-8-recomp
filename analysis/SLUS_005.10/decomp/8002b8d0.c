// addr: 0x8002b8d0  name: FUN_8002b8d0

void FUN_8002b8d0(int param_1)

{
  MATRIX MStack_30;
  SVECTOR local_10;
  
  if (iRam000008b0 != 0) {
    local_10.vx = -0x88;
    local_10.vy = 0;
    local_10.vz = *(short *)(param_1 + 0xa4) << 1;
    RotMatrixYXZ_gte(&local_10,&MStack_30);
    MStack_30.t[0] = -0x700;
    MStack_30.t[1] = 0x1800;
    MStack_30.t[2] = 0x2000;
    FUN_8001be5c(iRam000008b0,&MStack_30,uRam0000060c);
  }
  return;
}

