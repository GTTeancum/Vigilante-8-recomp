// addr: 0x8004fdb0  name: PutDispEnv

/* WARNING: Removing unreachable block (ram,0x8004ff04) */

DISPENV * PutDispEnv(DISPENV *env)

{
  short sVar1;
  bool bVar2;
  u_char uVar3;
  DISPENV *pDVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  uint uVar9;
  uint uVar10;
  int iVar11;
  uint uVar12;
  uint uVar13;
  int iVar14;
  
  uVar13 = 0x8000000;
  if (1 < DAT_80065026) {
    printf("PutDispEnv(%08x)...\n",env);
  }
  SYS_OBJ_20DC(((ushort)(env->disp).y & 0x3ff) << 10 | (ushort)(env->disp).x & 0x3ff | 0x5000000);
  iVar8._0_1_ = env->isinter;
  iVar8._1_1_ = env->isrgb24;
  iVar8._2_1_ = env->pad0;
  iVar8._3_1_ = env->pad1;
  if ((((iVar8 != 0) || ((env->disp).x != 0)) || ((env->disp).y != 0)) ||
     (((env->disp).w != 0 || ((env->disp).h != 0)))) {
    uVar3 = FUN_8004f1e8();
    env->pad0 = uVar3;
    if (uVar3 == '\x01') {
      uVar13 = 0x8000008;
    }
    if (env->isrgb24 != '\0') {
      uVar13 = uVar13 | 0x10;
    }
    if (env->isinter != '\0') {
      uVar13 = uVar13 | 0x20;
    }
    sVar1 = (env->disp).w;
    if (0x118 < sVar1) {
      if (sVar1 < 0x161) {
        pDVar4 = (DISPENV *)SYS_OBJ_D4C();
        return pDVar4;
      }
      if (sVar1 < 0x191) {
        pDVar4 = (DISPENV *)SYS_OBJ_D4C();
        return pDVar4;
      }
      if (sVar1 < 0x231) {
        pDVar4 = (DISPENV *)SYS_OBJ_D4C();
        return pDVar4;
      }
      uVar13 = uVar13 | 3;
    }
    sVar1 = (env->disp).h;
    bVar2 = sVar1 < 0x121;
    if (env->pad0 == '\0') {
      bVar2 = sVar1 < 0x101;
    }
    if (!bVar2) {
      uVar13 = uVar13 | 0x24;
    }
    SYS_OBJ_20DC(uVar13);
    env->pad0 = '\b';
  }
  if ((((env->screen).x != 0) || ((env->screen).y != 0)) ||
     (((env->screen).w != 0 || (((env->screen).h != 0 || (env->pad0 == '\b')))))) {
    uVar3 = FUN_8004f1e8();
    env->pad0 = uVar3;
    iVar8 = (int)(env->screen).y;
    iVar14 = iVar8 + 0x13;
    if (uVar3 == '\0') {
      iVar14 = iVar8 + 0x10;
    }
    iVar8 = (int)(env->screen).h;
    uVar13 = iVar14 + iVar8;
    if (iVar8 == 0) {
      uVar13 = iVar14 + 0xf0;
    }
    sVar1 = (env->disp).w;
    iVar8 = 0;
    if ((((0x118 < sVar1) && (iVar8 = 1, 0x160 < sVar1)) && (iVar8 = 2, 400 < sVar1)) &&
       (iVar8 = 4, sVar1 < 0x231)) {
      iVar8 = 3;
    }
    iVar5 = ((uint)env->pad0 * 5 + iVar8) * 4;
    iVar11 = (int)(env->screen).w;
    iVar6 = (uint)*(ushort *)(&DAT_800650fa + iVar5) - (uint)*(ushort *)(&DAT_800650f8 + iVar5);
    uVar9 = (uint)*(ushort *)(&DAT_800650f8 + iVar5) +
            (int)(env->screen).x * (uint)(byte)(&DAT_80065120)[iVar8];
    if (iVar11 != 0) {
      iVar6 = iVar6 * iVar11 >> 8;
    }
    uVar7 = uVar9 + iVar6;
    if (env->pad0 == '\0') {
      uVar10 = 500;
      if ((499 < (int)uVar9) && (uVar10 = 0xcb2, (int)uVar9 < 0xcb3)) {
        uVar10 = uVar9;
      }
      uVar9 = uVar10 + (uint)(byte)(&DAT_80065120)[iVar8] * 4;
      if (((int)uVar9 <= (int)uVar7) && (uVar9 = 0xcda, (int)uVar7 < 0xcdb)) {
        uVar9 = uVar7;
      }
      if (iVar14 < 0x10) {
        uVar7 = 0x10;
      }
      else {
        uVar7 = 0x101;
        if (iVar14 < 0x102) {
          pDVar4 = (DISPENV *)SYS_OBJ_FF4(uVar10,iVar14);
          return pDVar4;
        }
      }
      uVar12 = uVar7 + 2;
      if (((int)(uVar7 + 2) <= (int)uVar13) && (uVar12 = 0x102, (int)uVar13 < 0x103)) {
        uVar12 = uVar13;
      }
    }
    else {
      uVar10 = 0x21c;
      if ((0x21b < (int)uVar9) && (uVar10 = 0xc94, (int)uVar9 < 0xc95)) {
        uVar10 = uVar9;
      }
      uVar9 = uVar10 + (uint)(byte)(&DAT_80065120)[iVar8] * 4;
      if (((int)uVar9 <= (int)uVar7) && (uVar9 = 0xcbc, (int)uVar7 < 0xcbd)) {
        uVar9 = uVar7;
      }
      if (iVar14 < 0x13) {
        uVar7 = 0x13;
      }
      else {
        uVar7 = 0x12f;
        if (iVar14 < 0x130) {
          pDVar4 = (DISPENV *)SYS_OBJ_F64(uVar10,iVar14);
          return pDVar4;
        }
      }
      uVar12 = uVar7 + 2;
      if ((int)(uVar7 + 2) <= (int)uVar13) {
        pDVar4 = (DISPENV *)SYS_OBJ_100C(uVar10,0x131);
        return pDVar4;
      }
    }
    SYS_OBJ_20DC((uVar9 & 0xfff) << 0xc | uVar10 & 0xfff | 0x6000000);
    SYS_OBJ_20DC((uVar12 & 0x3ff) << 10 | uVar7 | 0x7000000);
  }
  FUN_80044c44(&DAT_80065090,env,0x14);
  return env;
}

