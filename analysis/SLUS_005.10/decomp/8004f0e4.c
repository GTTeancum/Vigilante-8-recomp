// addr: 0x8004f0e4  name: SetDefDrawEnv

DRAWENV * SetDefDrawEnv(DRAWENV *env,int x,int y,int w,int h)

{
  int iVar1;
  DRAWENV *pDVar2;
  
  iVar1 = FUN_8004f1e8();
  (env->clip).x = (short)x;
  (env->clip).y = (short)y;
  (env->clip).w = (short)w;
  (env->tw).x = 0;
  (env->tw).y = 0;
  (env->tw).w = 0;
  (env->tw).h = 0;
  env->r0 = '\0';
  env->g0 = '\0';
  env->b0 = '\0';
  env->dtd = '\x01';
  (env->clip).h = (short)h;
  if (iVar1 != 0) {
    pDVar2 = (DRAWENV *)EXT_OBJ_228();
    return pDVar2;
  }
  env->dfe = h < 0x101;
  env->ofs[0] = (short)x;
  env->ofs[1] = (short)y;
  env->tpage = 10;
  env->isbg = '\0';
  return env;
}

