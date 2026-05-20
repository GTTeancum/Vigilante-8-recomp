// addr: 0x800504a8  name: SetDrawEnv

void SetDrawEnv(DR_ENV *dr_env,DRAWENV *env)

{
  short sVar1;
  short sVar2;
  short sVar3;
  u_long uVar4;
  char cVar5;
  undefined4 local_18;
  undefined4 local_14;
  
  uVar4 = SYS_OBJ_172C((int)(env->clip).x,(int)(env->clip).y);
  dr_env->code[0] = uVar4;
  uVar4 = SYS_OBJ_17C4((int)(((uint)(ushort)(env->clip).w + (uint)(ushort)(env->clip).x + -1) *
                            0x10000) >> 0x10,
                       (int)(((uint)(ushort)(env->clip).y + (uint)(ushort)(env->clip).h + -1) *
                            0x10000) >> 0x10);
  dr_env->code[1] = uVar4;
  uVar4 = SYS_OBJ_185C((int)env->ofs[0],(int)env->ofs[1]);
  dr_env->code[2] = uVar4;
  uVar4 = SYS_OBJ_170C(env->dfe,env->dtd,env->tpage);
  dr_env->code[3] = uVar4;
  uVar4 = SYS_OBJ_1878(&env->tw);
  dr_env->code[4] = uVar4;
  dr_env->code[5] = 0xe6000000;
  cVar5 = '\a';
  if (env->isbg != '\0') {
    sVar3 = (env->clip).w;
    sVar1 = (env->clip).h;
    sVar2 = 0;
    if ((-1 < sVar3) && (sVar2 = DAT_80065028 + -1, (int)sVar3 <= DAT_80065028 + -1)) {
      sVar2 = sVar3;
    }
    if (sVar1 < 0) {
      sVar3 = 0;
    }
    else {
      sVar3 = DAT_8006502a + -1;
      if ((int)sVar1 <= DAT_8006502a + -1) {
        SYS_OBJ_1400(sVar1);
        return;
      }
    }
    cVar5 = '\n';
    local_14 = CONCAT22(sVar3,sVar2);
    local_18 = CONCAT22((env->clip).y - env->ofs[1],(env->clip).x - env->ofs[0]);
    dr_env->code[6] = (uint)env->b0 << 0x10 | (uint)env->g0 << 8 | 0x60000000 | (uint)env->r0;
    dr_env->code[7] = local_18;
    dr_env->code[8] = local_14;
  }
  *(char *)((int)&dr_env->tag + 3) = cVar5 + -1;
  return;
}

