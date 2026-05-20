// addr: 0x8004fd7c  name: GetDrawEnv

DRAWENV * GetDrawEnv(DRAWENV *env)

{
  FUN_80044c44(env,&DAT_80065034,0x5c);
  return env;
}

