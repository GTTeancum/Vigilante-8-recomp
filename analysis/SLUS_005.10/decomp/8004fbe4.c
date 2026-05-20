// addr: 0x8004fbe4  name: PutDrawEnv

DRAWENV * PutDrawEnv(DRAWENV *env)

{
  if (1 < DAT_80065026) {
    printf("PutDrawEnv(%08x)...\n",env);
  }
  SYS_OBJ_149C(&env->dr_env,env);
  (env->dr_env).tag = (env->dr_env).tag | 0xffffff;
  SYS_OBJ_21F0(SYS_OBJ_2154,&env->dr_env,0x40,0);
  FUN_80044c44(&DAT_80065034,env,0x5c);
  return env;
}

