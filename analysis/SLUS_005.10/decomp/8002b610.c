// addr: 0x8002b610  name: FUN_8002b610

void FUN_8002b610(undefined4 param_1,int param_2,undefined4 param_3,undefined4 param_4,uint param_5,
                 undefined4 param_6)

{
  int iVar1;
  short local_30;
  short local_2e;
  undefined2 local_2c;
  short local_2a;
  short local_28;
  undefined2 local_26;
  undefined2 local_24;
  undefined2 local_22;
  undefined2 local_20;
  int local_1c;
  int local_18;
  int local_14;
  
  FUN_80043408(param_4,param_1,&local_1c);
  local_1c = (int)(local_1c * param_5) >> 8;
  local_18 = (int)(local_18 * param_5) >> 8;
  local_14 = (int)(local_14 * param_5) >> 8;
  if (((((local_1c - param_2) * 0x100 < local_14 * 0xa0) &&
       (local_14 * -0xa0 < (local_1c + param_2) * 0x100)) &&
      ((local_18 - param_2) * 0x100 < local_14 * 0x78)) &&
     (local_14 * -0x78 < (local_18 + param_2) * 0x100)) {
    iVar1 = (short)(&DAT_800607b6)[(param_5 & 0x1ff) * 0x10] * param_2;
    local_30 = (short)((uint)iVar1 >> 0x10);
    if (iVar1 < 0) {
      local_30 = (short)((uint)(iVar1 + 0xfffff) >> 0x10);
    }
    local_30 = local_30 >> 4;
    param_2 = (short)(&DAT_800607b4)[(param_5 & 0x1ff) * 0x10] * param_2;
    if (param_2 < 0) {
      param_2 = param_2 + 0xfffff;
    }
    local_2a = (short)(param_2 >> 0x14);
    local_2e = -local_2a;
    local_20 = 0x1000;
    local_22 = 0;
    local_24 = 0;
    local_26 = 0;
    local_2c = 0;
    local_28 = local_30;
    FUN_8001be5c(param_3,&local_30,param_6);
  }
  return;
}

