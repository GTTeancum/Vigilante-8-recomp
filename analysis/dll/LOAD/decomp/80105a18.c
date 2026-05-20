// addr: 0x80105a18  name: FUN_80105a18

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_80105a18(undefined4 param_1)

{
  undefined2 uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  undefined1 auStack_28 [12];
  int local_1c;
  
  FUN_800187e4/*0x800187e4*/(param_1,&DAT_80065b18);
  FUN_800185cc/*0x800185cc*/(auStack_28,param_1);
  iVar4 = 0;
  _DAT_8005e9b6 = (*(short *)(local_1c + 2) - *(short *)(local_1c + 6)) * 0x10;
  iVar2 = -0x7ff6efe0;
  iVar3 = -0x7ff6efb8;
  _DAT_8005e9ce = *(short *)(local_1c + 2) << 4;
  _DAT_8005e9be = _DAT_8005e9b6;
  _DAT_8005e9c6 = _DAT_8005e9b6;
  _DAT_8005e9d6 = _DAT_8005e9ce;
  _DAT_8005e9de = _DAT_8005e9ce;
  do {
    *(undefined1 *)(iVar2 + 3) = 9;
    *(undefined1 *)(iVar2 + 7) = 0x2c;
    *(undefined1 *)(iVar3 + 3) = 9;
    *(undefined1 *)(iVar3 + 7) = 0x2c;
    *(byte *)(iVar2 + 7) = *(byte *)(iVar2 + 7) | 1;
    *(byte *)(iVar3 + 7) = *(byte *)(iVar3 + 7) | 1;
    uVar1 = _DAT_80065b22;
    *(undefined2 *)(iVar2 + 0x36) = _DAT_80065b22;
    *(undefined2 *)(iVar2 + 0xe) = uVar1;
    uVar1 = _DAT_80065b20;
    *(undefined2 *)(iVar2 + 0x3e) = _DAT_80065b20;
    *(undefined2 *)(iVar2 + 0x16) = uVar1;
    *(char *)(iVar2 + 0xc) = DAT_80065b1e;
    *(char *)(iVar2 + 0xd) = (char)((uint)_DAT_80065b1c >> 0x18);
    *(char *)(iVar2 + 0x14) = DAT_80065b1a + DAT_80065b1e + -1;
    *(char *)(iVar2 + 0x15) = (char)((uint)_DAT_80065b1c >> 0x18);
    *(char *)(iVar2 + 0x1c) = DAT_80065b1e;
    *(char *)(iVar2 + 0x1d) = DAT_80065b1c + (char)((uint)_DAT_80065b1c >> 0x18) + -1;
    *(char *)(iVar2 + 0x24) = DAT_80065b1a + DAT_80065b1e + -1;
    *(char *)(iVar2 + 0x25) = DAT_80065b1c + (char)((uint)_DAT_80065b1c >> 0x18) + -1;
    *(char *)(iVar3 + 0xc) = DAT_80065b1e;
    *(char *)(iVar3 + 0xd) = (char)((uint)_DAT_80065b1c >> 0x18);
    *(char *)(iVar3 + 0x14) = DAT_80065b1a + DAT_80065b1e + -1;
    *(char *)(iVar3 + 0x15) = (char)((uint)_DAT_80065b1c >> 0x18);
    *(char *)(iVar3 + 0x1c) = DAT_80065b1e;
    *(char *)(iVar3 + 0x1d) = DAT_80065b1c + (char)((uint)_DAT_80065b1c >> 0x18) + -1;
    *(char *)(iVar3 + 0x24) = DAT_80065b1a + DAT_80065b1e + -1;
    iVar4 = iVar4 + 1;
    iVar2 = iVar2 + 0x50;
    *(char *)(iVar3 + 0x25) = DAT_80065b1c + (char)((uint)_DAT_80065b1c >> 0x18) + -1;
    iVar3 = iVar3 + 0x50;
  } while (iVar4 < 2);
  if (_DAT_80065a28 == 0) {
    _DAT_80065a28 = _DAT_80065b18;
    _DAT_80065a2c = _DAT_80065b1c;
    _DAT_80065a30 = _DAT_80065b20;
    DAT_800659d2 = DAT_80065b1c;
  }
  DrawSync/*0x8004f580*/(0);
  return;
}

