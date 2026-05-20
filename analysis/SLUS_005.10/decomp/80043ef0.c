// addr: 0x80043ef0  name: FUN_80043ef0

void FUN_80043ef0(void)

{
  undefined1 *puVar1;
  int iVar2;
  SpuCommonAttr local_78;
  SpuVoiceAttr local_50;
  
  SsUtReverbOff();
  SpuInitMalloc(0x10,&DAT_800a3008);
  iVar2 = 0x17;
  puVar1 = &DAT_800a3007;
  do {
    *puVar1 = (char)iVar2;
    iVar2 = iVar2 + -1;
    puVar1 = puVar1 + -1;
  } while (-1 < iVar2);
  CdMix((CdlATV *)&DAT_800658b4);
  local_78.mask = 0x2c3;
  local_78.cd.volume.right = 0x3fff;
  local_78.cd.volume.left = 0x3fff;
  local_78.mvol.right = 0x3fff;
  local_78.mvol.left = 0x3fff;
  local_78.cd.mix = 1;
  SpuSetCommonAttr(&local_78);
  FUN_80044080(0,0x2ccc,0x2ccc);
  local_50.voice = 0xffffff;
  local_50.mask = 0xff13;
  local_50.pitch = 0x400;
  local_50.r_mode = 3;
  local_50.volume.left = 0x3fff;
  local_50.volume.right = 0x3fff;
  local_50.a_mode = 1;
  local_50.s_mode = 1;
  local_50.ar = 0;
  local_50.dr = 0;
  local_50.sr = 0;
  local_50.rr = 0;
  local_50.sl = 0xf;
  SpuSetVoiceAttr(&local_50);
  uRam000008fc = 0;
  FUN_80043a74();
  return;
}

