// addr: 0x800441f8  name: FUN_800441f8

short * FUN_800441f8(void)

{
  uint uVar1;
  ulong addr;
  short *psVar2;
  long lVar3;
  short *psVar4;
  ulong size;
  int iVar5;
  uint uVar6;
  short local_420;
  ushort local_41e;
  uchar auStack_418 [1024];
  
  FUN_80015a20(&local_420,4);
  addr = SpuMalloc((uint)local_41e * 8 + 0x3f & 0xffffffc0);
  if (addr == 0) {
    FUN_80015368("Out of SPU RAM");
  }
  psVar2 = (short *)FUN_800116f4(local_420 * 4 + 4);
  psVar2[1] = (short)(addr >> 3);
  *psVar2 = local_420;
  FUN_80015a20(psVar2 + 2,(int)local_420 << 2);
  iVar5 = 0;
  psVar4 = psVar2;
  if (0 < local_420) {
    do {
      psVar4[2] = psVar4[2] + psVar2[1];
      iVar5 = iVar5 + 1;
      psVar4 = psVar4 + 2;
    } while (iVar5 < local_420);
  }
  SpuSetTransferMode(0);
  SpuSetTransferStartAddr(addr);
  uVar6 = (uint)local_41e << 3;
  uVar1 = (uint)local_41e;
  while (uVar1 != 0) {
    size = 0x400;
    if ((int)uVar6 < 0x400) {
      size = uVar6;
    }
    FUN_80015a20(auStack_418);
    SpuSetTransferStartAddr(addr);
    SpuRead(auStack_418,size);
    do {
      lVar3 = SpuIsTransferCompleted(0);
    } while (lVar3 == 0);
    addr = addr + size;
    uVar6 = uVar6 - size;
    uVar1 = uVar6;
  }
  return psVar2;
}

