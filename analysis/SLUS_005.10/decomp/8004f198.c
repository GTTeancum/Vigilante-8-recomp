// addr: 0x8004f198  name: SetDefDispEnv

DISPENV * SetDefDispEnv(DISPENV *env,int x,int y,int w,int h)

{
  (env->disp).x = (short)x;
  (env->disp).y = (short)y;
  (env->disp).w = (short)w;
  (env->screen).x = 0;
  (env->screen).y = 0;
  (env->screen).w = 0;
  (env->screen).h = 0;
  env->isrgb24 = '\0';
  env->isinter = '\0';
  env->pad1 = '\0';
  env->pad0 = '\0';
  (env->disp).h = (short)h;
  return env;
}

