// addr: 0x80044484  name: FUN_80044484

void FUN_80044484(int param_1,int param_2,int param_3,undefined4 param_4)

{
  int iVar1;
  uint vNum;
  
  if (param_1 != 0) {
    iVar1 = FUN_80017160();
    param_2 = param_2 + param_3 * 4;
    vNum = param_1 - 1;
    SpuSetVoicePitch(vNum,(ushort)((int)*(short *)(param_2 + 6) * (iVar1 + 0x1c000) >> 0x11));
    SpuSetVoiceVolume(vNum,(short)param_4,(short)((uint)param_4 >> 0x10));
    SpuSetVoiceStartAddr(vNum,(uint)*(ushort *)(param_2 + 4) << 3);
    FUN_80043ff0(1,1 << (vNum & 0x1f));
    *(undefined1 *)((int)&DAT_800a2fec + param_1 + 3) = uRamffffacb0;
  }
  return;
}

