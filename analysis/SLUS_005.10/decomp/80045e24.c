// addr: 0x80045e24  name: _spu_FsetRXXa

uint _spu_FsetRXXa(int param_1,uint param_2)

{
  uint uVar1;
  
  if (DAT_8005edf8 != 0) {
    if (DAT_8005ee00 == 0) {
      trap(0x1c00);
    }
    if (param_2 % DAT_8005ee00 != 0) {
      param_2 = param_2 + DAT_8005ee00 & ~DAT_8005ee04;
    }
  }
  if (param_1 != -2) {
    if (param_1 == -1) {
      uVar1 = SPU_OBJ_9EC();
      return uVar1;
    }
    *(short *)((int)&VOICE_00_LEFT_RIGHT + param_1 * 2) = (short)(param_2 >> (DAT_8005edfc & 0x1f));
    return param_2;
  }
  uVar1 = SPU_OBJ_9EC();
  return uVar1;
}

