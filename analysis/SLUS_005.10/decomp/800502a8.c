// addr: 0x800502a8  name: GetDispEnv

DISPENV * GetDispEnv(DISPENV *env)

{
  FUN_80044c44(env,&DAT_80065090,0x14);
  return env;
}

