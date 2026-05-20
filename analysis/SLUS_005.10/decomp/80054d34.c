// addr: 0x80054d34  name: _padRecvAtLoadInfo

undefined4 _padRecvAtLoadInfo(int param_1)

{
  byte bVar1;
  undefined1 uVar2;
  byte bVar3;
  undefined4 uVar4;
  int iVar5;
  
  bVar1 = *(byte *)(param_1 + 0x46);
  if (bVar1 == 3) {
    bVar1 = *(byte *)(*(int *)(param_1 + 0x3c) + 4);
    bVar3 = *(byte *)(*(int *)(param_1 + 0x3c) + 5);
    *(undefined1 *)(param_1 + 0x47) = 0;
    *(ushort *)(param_1 + 0xe6) = (ushort)bVar3 + (ushort)bVar1 * 0x100;
    uVar4 = PADCMD_OBJ_1D8();
    return uVar4;
  }
  if (bVar1 < 4) {
    if (bVar1 != 2) {
      uVar4 = PADCMD_OBJ_1DC();
      return uVar4;
    }
    *(undefined1 *)(param_1 + 0xe3) = *(undefined1 *)(*(int *)(param_1 + 0x3c) + 3);
    uVar2 = *(undefined1 *)(*(int *)(param_1 + 0x3c) + 4);
    *(undefined2 *)(param_1 + 0xe6) = 0;
    *(undefined1 *)(param_1 + 0xe4) = uVar2;
    *(undefined1 *)(param_1 + 0xe9) = *(undefined1 *)(*(int *)(param_1 + 0x3c) + 5);
    uVar2 = *(undefined1 *)(*(int *)(param_1 + 0x3c) + 6);
    *(undefined4 *)(param_1 + 0xec) = 0;
    *(undefined1 *)(param_1 + 0xea) = uVar2;
    uVar4 = PADCMD_OBJ_1D8();
    return uVar4;
  }
  if (bVar1 != 4) {
    uVar4 = PADCMD_OBJ_1DC();
    return uVar4;
  }
  bVar3 = *(char *)(param_1 + 0x47) + 1;
  bVar1 = *(byte *)(*(int *)(param_1 + 0x3c) + 4);
  *(byte *)(param_1 + 0x47) = bVar3;
  *(uint *)(param_1 + 0xec) = *(int *)(param_1 + 0xec) + 8 + (bVar1 + 3 & 0x1fc);
  if (bVar3 < *(byte *)(param_1 + 0xea)) {
    return 0;
  }
  iVar5 = _padGetActSize(param_1);
  if (0x80 < iVar5) {
    *(undefined1 *)(param_1 + 0x46) = 0xfe;
    *(undefined1 *)(param_1 + 0x49) = 2;
    uVar4 = PADCMD_OBJ_18C();
    return uVar4;
  }
  *(undefined1 *)(param_1 + 0x46) = 0xff;
  _padLoadActInfo(param_1,param_1 + 99);
  *(undefined1 *)(param_1 + 0x46) = 2;
  uVar4 = PADCMD_OBJ_18C();
  return uVar4;
}

