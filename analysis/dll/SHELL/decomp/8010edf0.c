// addr: 0x8010edf0  name: FUN_8010edf0

undefined4 FUN_8010edf0(byte param_1)

{
  uint uVar1;
  
  uVar1 = (uint)param_1;
  if ((*(byte *)(uVar1 + 0x80065175) & 4) != 0) {
    param_1 = 1;
  }
  if ((*(byte *)(uVar1 + 0x80065175) & 3) != 0) {
    tolower/*0x80052d18*/((int)(char)param_1);
  }
  return 9999999;
}

