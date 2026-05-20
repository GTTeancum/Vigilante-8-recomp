// addr: 0x80046438  name: S_M_INT_OBJ_C4

void S_M_INT_OBJ_C4(void)

{
  uint *puVar1;
  uint *puVar2;
  uint *puVar3;
  uint uVar4;
  int iVar5;
  uint *in_a3;
  uint uVar6;
  int in_t0;
  int in_t1;
  int iVar7;
  int iVar8;
  uint in_t2;
  uint in_t3;
  uint in_t4;
  int in_t5;
  
  iVar8 = DAT_8005ee38;
  for (; in_t1 <= in_t5; in_t1 = in_t1 + 1) {
    iVar7 = in_t1 + 1;
    if ((*in_a3 & in_t4) != 0) {
      puVar1 = (uint *)(iVar7 * 8 + in_t0);
      while (uVar4 = *puVar1, puVar1 = puVar1 + 2, uVar4 == in_t2) {
        iVar7 = iVar7 + 1;
      }
      puVar1 = (uint *)(iVar7 * 8 + in_t0);
      if (((*puVar1 & in_t4) != 0) && ((*puVar1 & in_t3) == (*in_a3 & in_t3) + in_a3[1])) {
        *puVar1 = in_t2;
        in_a3[1] = in_a3[1] + puVar1[1];
        S_M_INT_OBJ_C4();
        return;
      }
    }
    in_a3 = in_a3 + 2;
  }
  iVar7 = 0;
  puVar1 = DAT_8005ee3c;
  if (-1 < DAT_8005ee38) {
    do {
      if (puVar1[1] == 0) {
        *puVar1 = 0x2fffffff;
      }
      iVar7 = iVar7 + 1;
      puVar1 = puVar1 + 2;
    } while (iVar7 <= iVar8);
  }
  puVar1 = DAT_8005ee3c;
  iVar8 = 0;
  puVar3 = DAT_8005ee3c;
  if (-1 < DAT_8005ee38) {
    do {
      iVar7 = DAT_8005ee38;
      if ((*puVar3 & 0x40000000) != 0) break;
      iVar5 = iVar8 + 1;
      if (iVar5 <= DAT_8005ee38) {
        puVar2 = puVar1 + iVar5 * 2;
        do {
          uVar4 = *puVar2;
          if ((uVar4 & 0x40000000) != 0) break;
          uVar6 = *puVar3;
          if ((uVar4 & 0xfffffff) < (uVar6 & 0xfffffff)) {
            *puVar3 = uVar4;
            uVar4 = puVar3[1];
            puVar3[1] = puVar2[1];
            *puVar2 = uVar6;
            puVar2[1] = uVar4;
          }
          iVar5 = iVar5 + 1;
          puVar2 = puVar2 + 2;
        } while (iVar5 <= iVar7);
      }
      iVar8 = iVar8 + 1;
      puVar3 = puVar3 + 2;
    } while (iVar8 <= DAT_8005ee38);
  }
  puVar1 = DAT_8005ee3c;
  iVar8 = 0;
  puVar3 = DAT_8005ee3c;
  if (-1 < DAT_8005ee38) {
    do {
      if ((*puVar3 & 0x40000000) != 0) break;
      if (*puVar3 == 0x2fffffff) {
        puVar1 = DAT_8005ee3c + DAT_8005ee38 * 2;
        *puVar3 = *puVar1;
        DAT_8005ee38 = iVar8;
        puVar3[1] = puVar1[1];
        S_M_INT_OBJ_270();
        return;
      }
      iVar8 = iVar8 + 1;
      puVar3 = puVar3 + 2;
    } while (iVar8 <= DAT_8005ee38);
  }
  iVar8 = DAT_8005ee38 + -1;
  if (-1 < iVar8) {
    puVar3 = DAT_8005ee3c + iVar8 * 2;
    do {
      if ((*puVar3 & 0x80000000) == 0) {
        return;
      }
      *puVar3 = *puVar3 & 0xfffffff | 0x40000000;
      iVar7 = DAT_8005ee38 * 2;
      iVar5 = iVar8 + -1;
      DAT_8005ee38 = iVar8;
      puVar3[1] = puVar3[1] + puVar1[iVar7 + 1];
      puVar3 = puVar3 + -2;
      iVar8 = iVar5;
    } while (-1 < iVar5);
  }
  return;
}

