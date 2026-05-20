// addr: 0x80050c8c  name: SYS_OBJ_1A88

undefined4 SYS_OBJ_1A88(void)

{
  uint uVar1;
  undefined4 uVar2;
  ushort in_v1;
  ushort *in_t0;
  uint in_t1;
  
  in_t0[3] = in_v1;
  if (((*in_t0 & 0x3f) == 0) && ((in_t0[2] & 0x3f) == 0)) {
    DAT_800a32f8 = 0x5ffffff;
    DAT_800a32fc = 0xe6000000;
    DAT_800a3304 = in_t1 & 0xffffff | 0x2000000;
                    /* Probable PsyQ macro: setDrawTPage() if setlen(p, 1), setDrawMode() if
                       setlen(p, 2). */
    DAT_800a3300 = GPU_REG1 & 0x7ff | (in_t1 >> 0x1f) << 10 | 0xe1000000;
    DAT_800a3308 = *(undefined4 *)in_t0;
    DAT_800a330c = *(uint *)(in_t0 + 2);
    SYS_OBJ_2154(&DAT_800a32f8);
    return 0;
  }
  DAT_800a32f8 = 0x80a3320;
  DAT_800a3310 = in_t1 & 0xffffff | 0x60000000;
  DAT_800a3308 = 0xe6000000;
  DAT_800a32fc = 0xe3000000;
  DAT_800a3300 = 0xe4ffffff;
  DAT_800a3304 = 0xe5000000;
                    /* Probable PsyQ macro: setDrawTPage() if setlen(p, 1), setDrawMode() if
                       setlen(p, 2). */
  DAT_800a330c = GPU_REG1 & 0x7ff | (in_t1 >> 0x1f) << 10 | 0xe1000000;
  DAT_800a3314 = *(undefined4 *)in_t0;
  DAT_800a3318 = *(undefined4 *)(in_t0 + 2);
  DAT_800a3320 = 0x3ffffff;
  uVar1 = SYS_OBJ_219C(3);
  DAT_800a3324 = uVar1 | 0xe3000000;
  uVar1 = SYS_OBJ_219C(4);
  DAT_800a3328 = uVar1 | 0xe4000000;
  uVar1 = SYS_OBJ_219C(5);
  DAT_800a332c = uVar1 | 0xe5000000;
  uVar2 = SYS_OBJ_1BF8();
  return uVar2;
}

