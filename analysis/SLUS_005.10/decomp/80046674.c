// addr: 0x80046674  name: SpuFree

void SpuFree(ulong addr)

{
  uint *puVar1;
  int iVar2;
  
  iVar2 = 0;
  if (0 < DAT_8005ee34) {
    puVar1 = DAT_8005ee3c;
    do {
      if ((*puVar1 & 0x40000000) != 0) break;
      iVar2 = iVar2 + 1;
      if (*puVar1 == addr) {
        *puVar1 = addr | 0x80000000;
        S_M_F_OBJ_64();
        return;
      }
      puVar1 = puVar1 + 2;
    } while (iVar2 < DAT_8005ee34);
  }
  _spu_gcSPU();
  return;
}

