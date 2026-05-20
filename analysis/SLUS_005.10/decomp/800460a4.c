// addr: 0x800460a4  name: SpuMalloc

/* WARNING: Removing unreachable block (ram,0x800461dc) */
/* WARNING: Removing unreachable block (ram,0x800462a0) */
/* WARNING: Removing unreachable block (ram,0x800462b4) */
/* WARNING: Removing unreachable block (ram,0x800462d4) */
/* WARNING: Removing unreachable block (ram,0x80046310) */
/* WARNING: Removing unreachable block (ram,0x80046200) */
/* WARNING: Removing unreachable block (ram,0x80046218) */
/* WARNING: Removing unreachable block (ram,0x80046230) */

long SpuMalloc(long size)

{
  uint uVar1;
  uint uVar2;
  long lVar3;
  uint *puVar4;
  int iVar5;
  
  iVar5 = 0;
  if (DAT_8005ed6c == 0) {
    lVar3 = S_M_M_OBJ_54();
    return lVar3;
  }
  if ((size & ~DAT_8005ee04) != 0) {
    size = size + DAT_8005ee04;
  }
  uVar2 = DAT_8005edfc & 0x1f;
  uVar1 = DAT_8005edfc & 0x1f;
  if ((*DAT_8005ee3c & 0x40000000) != 0) {
    lVar3 = S_M_M_OBJ_12C();
    return lVar3;
  }
  _spu_gcSPU();
  puVar4 = DAT_8005ee3c;
  if (0 < DAT_8005ee34) {
    do {
      if (((*puVar4 & 0x40000000) != 0) ||
         (((*puVar4 & 0x80000000) != 0 && ((uint)((size >> uVar2) << uVar1) <= puVar4[1])))) {
        lVar3 = S_M_M_OBJ_12C();
        return lVar3;
      }
      iVar5 = iVar5 + 1;
      puVar4 = puVar4 + 2;
    } while (iVar5 < DAT_8005ee34);
  }
  return -1;
}

