// addr: 0x80012828  name: FUN_80012828

void FUN_80012828(undefined4 param_1,DRAWENV *param_2,u_long *param_3,uint *param_4)

{
  uint uVar1;
  
  uRam0000016c = 0;
  uRam00000168 = 0;
  uRam000005d8 = param_1;
  puRam000005dc = param_4;
  SetDrawEnv(&param_2->dr_env,param_2);
  uVar1 = *param_4;
  *param_4 = (uint)&param_2->dr_env & 0xffffff;
  (param_2->dr_env).tag = (uint)*(byte *)((int)&(param_2->dr_env).tag + 3) << 0x18 | uVar1;
  DrawSyncCallback(FUN_800127f4);
  DrawOTag(param_3);
  return;
}

