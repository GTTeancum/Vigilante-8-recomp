// addr: 0x80050bf4  name: SYS_OBJ_19F0

undefined4 SYS_OBJ_19F0(ushort *param_1,uint param_2)

{
  ushort uVar1;
  undefined4 uVar2;
  uint uVar3;
  
  if ((short)param_1[2] < 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = DAT_80065028 - 1;
    if ((int)(short)param_1[2] <= DAT_80065028 + -1) {
      uVar2 = SYS_OBJ_1A48(param_1[2]);
      return uVar2;
    }
  }
  param_1[2] = uVar1;
  if ((short)param_1[3] < 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = DAT_8006502a - 1;
    if ((int)(short)param_1[3] <= DAT_8006502a + -1) {
      uVar2 = SYS_OBJ_1A88(param_1[3]);
      return uVar2;
    }
  }
  param_1[3] = uVar1;
  if (((*param_1 & 0x3f) == 0) && ((param_1[2] & 0x3f) == 0)) {
    DAT_800a32f8 = 0x5ffffff;
    DAT_800a32fc = 0xe6000000;
    DAT_800a3304 = param_2 & 0xffffff | 0x2000000;
    DAT_800a3300 = GPU_REG1 & 0x7ff | (param_2 >> 0x1f) << 10 | 0xe1000000;
    DAT_800a3308 = *(undefined4 *)param_1;
    DAT_800a330c = *(uint *)(param_1 + 2);
    SYS_OBJ_2154(&DAT_800a32f8);
    return 0;
  }
  DAT_800a32f8 = 0x80a3320;
  DAT_800a3310 = param_2 & 0xffffff | 0x60000000;
  DAT_800a3308 = 0xe6000000;
  DAT_800a32fc = 0xe3000000;
  DAT_800a3300 = 0xe4ffffff;
  DAT_800a3304 = 0xe5000000;
  DAT_800a330c = GPU_REG1 & 0x7ff | (param_2 >> 0x1f) << 10 | 0xe1000000;
  DAT_800a3314 = *(undefined4 *)param_1;
  DAT_800a3318 = *(undefined4 *)(param_1 + 2);
  DAT_800a3320 = 0x3ffffff;
  uVar3 = SYS_OBJ_219C(3);
  DAT_800a3324 = uVar3 | 0xe3000000;
  uVar3 = SYS_OBJ_219C(4);
  DAT_800a3328 = uVar3 | 0xe4000000;
  uVar3 = SYS_OBJ_219C(5);
  DAT_800a332c = uVar3 | 0xe5000000;
  uVar2 = SYS_OBJ_1BF8();
  return uVar2;
}

