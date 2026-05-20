// addr: 0x80046374  name: _spu_gcSPU

void _spu_gcSPU(void)

{
  uint uVar1;
  uint *puVar2;
  uint *puVar3;
  uint *puVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  
  iVar8 = DAT_8005ee38;
  iVar7 = 0;
  puVar3 = DAT_8005ee3c;
  if (-1 < DAT_8005ee38) {
    do {
      iVar5 = iVar7 + 1;
      if ((*puVar3 & 0x80000000) != 0) {
        puVar2 = DAT_8005ee3c + iVar5 * 2;
        while (uVar1 = *puVar2, puVar2 = puVar2 + 2, uVar1 == 0x2fffffff) {
          iVar5 = iVar5 + 1;
        }
        puVar2 = DAT_8005ee3c + iVar5 * 2;
        if (((*puVar2 & 0x80000000) != 0) &&
           ((*puVar2 & 0xfffffff) == (*puVar3 & 0xfffffff) + puVar3[1])) {
          *puVar2 = 0x2fffffff;
          puVar3[1] = puVar3[1] + puVar2[1];
          S_M_INT_OBJ_C4();
          return;
        }
      }
      iVar7 = iVar7 + 1;
      puVar3 = puVar3 + 2;
    } while (iVar7 <= DAT_8005ee38);
  }
  iVar7 = 0;
  puVar3 = DAT_8005ee3c;
  if (-1 < DAT_8005ee38) {
    do {
      if (puVar3[1] == 0) {
        *puVar3 = 0x2fffffff;
      }
      iVar7 = iVar7 + 1;
      puVar3 = puVar3 + 2;
    } while (iVar7 <= iVar8);
  }
  puVar3 = DAT_8005ee3c;
  iVar8 = 0;
  puVar2 = DAT_8005ee3c;
  if (-1 < DAT_8005ee38) {
    do {
      iVar7 = DAT_8005ee38;
      if ((*puVar2 & 0x40000000) != 0) break;
      iVar5 = iVar8 + 1;
      if (iVar5 <= DAT_8005ee38) {
        puVar4 = puVar3 + iVar5 * 2;
        do {
          uVar1 = *puVar4;
          if ((uVar1 & 0x40000000) != 0) break;
          uVar6 = *puVar2;
          if ((uVar1 & 0xfffffff) < (uVar6 & 0xfffffff)) {
            *puVar2 = uVar1;
            uVar1 = puVar2[1];
            puVar2[1] = puVar4[1];
            *puVar4 = uVar6;
            puVar4[1] = uVar1;
          }
          iVar5 = iVar5 + 1;
          puVar4 = puVar4 + 2;
        } while (iVar5 <= iVar7);
      }
      iVar8 = iVar8 + 1;
      puVar2 = puVar2 + 2;
    } while (iVar8 <= DAT_8005ee38);
  }
  puVar3 = DAT_8005ee3c;
  iVar8 = 0;
  puVar2 = DAT_8005ee3c;
  if (-1 < DAT_8005ee38) {
    do {
      if ((*puVar2 & 0x40000000) != 0) break;
      if (*puVar2 == 0x2fffffff) {
        puVar3 = DAT_8005ee3c + DAT_8005ee38 * 2;
        *puVar2 = *puVar3;
        DAT_8005ee38 = iVar8;
        puVar2[1] = puVar3[1];
        S_M_INT_OBJ_270();
        return;
      }
      iVar8 = iVar8 + 1;
      puVar2 = puVar2 + 2;
    } while (iVar8 <= DAT_8005ee38);
  }
  iVar8 = DAT_8005ee38 + -1;
  if (-1 < iVar8) {
    puVar2 = DAT_8005ee3c + iVar8 * 2;
    do {
      if ((*puVar2 & 0x80000000) == 0) {
        return;
      }
      *puVar2 = *puVar2 & 0xfffffff | 0x40000000;
      iVar7 = DAT_8005ee38 * 2;
      iVar5 = iVar8 + -1;
      DAT_8005ee38 = iVar8;
      puVar2[1] = puVar2[1] + puVar3[iVar7 + 1];
      puVar2 = puVar2 + -2;
      iVar8 = iVar5;
    } while (-1 < iVar5);
  }
  return;
}

