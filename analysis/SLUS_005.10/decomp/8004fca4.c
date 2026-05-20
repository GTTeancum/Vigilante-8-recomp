// addr: 0x8004fca4  name: DrawOTagEnv

void DrawOTagEnv(u_long *p,DRAWENV *env)

{
  if (1 < DAT_80065026) {
    printf("DrawOTagEnv(%08x,&08x)...\n",p,env);
  }
  SYS_OBJ_149C(&env->dr_env,env);
  (env->dr_env).tag = (env->dr_env).tag & 0xff000000 | (uint)p & 0xffffff;
  SYS_OBJ_21F0(SYS_OBJ_2154,&env->dr_env,0x40,0);
  FUN_80044c44(&DAT_80065034,env,0x5c);
  return;
}

