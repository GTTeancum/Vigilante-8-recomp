// addr: 0x800463c8  name: S_M_INT_OBJ_54

void S_M_INT_OBJ_54(undefined4 param_1,undefined4 param_2,int param_3,uint *param_4)

{
  uint uVar1;
  uint *in_v1;
  uint *puVar2;
  uint *puVar3;
  uint *puVar4;
  int iVar5;
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
code_r0x800463c8:
  while (uVar1 = *in_v1, in_v1 = in_v1 + 2, uVar1 == in_t2) {
    param_3 = param_3 + 1;
  }
  puVar4 = (uint *)(param_3 * 8 + in_t0);
  iVar7 = in_t1;
  if (((*puVar4 & in_t4) != 0) && ((*puVar4 & in_t3) == (*param_4 & in_t3) + param_4[1])) {
    *puVar4 = in_t2;
    param_4[1] = param_4[1] + puVar4[1];
    S_M_INT_OBJ_C4();
    return;
  }
  while( true ) {
    param_4 = param_4 + 2;
    in_t1 = iVar7 + 1;
    if (in_t5 < in_t1) break;
    param_3 = iVar7 + 2;
    iVar7 = in_t1;
    if ((*param_4 & in_t4) != 0) goto code_r0x800463c0;
  }
  iVar7 = 0;
  puVar4 = DAT_8005ee3c;
  if (-1 < DAT_8005ee38) {
    do {
      if (puVar4[1] == 0) {
        *puVar4 = 0x2fffffff;
      }
      iVar7 = iVar7 + 1;
      puVar4 = puVar4 + 2;
    } while (iVar7 <= iVar8);
  }
  puVar4 = DAT_8005ee3c;
  iVar8 = 0;
  puVar3 = DAT_8005ee3c;
  if (-1 < DAT_8005ee38) {
    do {
      iVar7 = DAT_8005ee38;
      if ((*puVar3 & 0x40000000) != 0) break;
      iVar5 = iVar8 + 1;
      if (iVar5 <= DAT_8005ee38) {
        puVar2 = puVar4 + iVar5 * 2;
        do {
          uVar1 = *puVar2;
          if ((uVar1 & 0x40000000) != 0) break;
          uVar6 = *puVar3;
          if ((uVar1 & 0xfffffff) < (uVar6 & 0xfffffff)) {
            *puVar3 = uVar1;
            uVar1 = puVar3[1];
            puVar3[1] = puVar2[1];
            *puVar2 = uVar6;
            puVar2[1] = uVar1;
          }
          iVar5 = iVar5 + 1;
          puVar2 = puVar2 + 2;
        } while (iVar5 <= iVar7);
      }
      iVar8 = iVar8 + 1;
      puVar3 = puVar3 + 2;
    } while (iVar8 <= DAT_8005ee38);
  }
  puVar4 = DAT_8005ee3c;
  iVar8 = 0;
  puVar3 = DAT_8005ee3c;
  if (-1 < DAT_8005ee38) {
    do {
      if ((*puVar3 & 0x40000000) != 0) break;
      if (*puVar3 == 0x2fffffff) {
        puVar4 = DAT_8005ee3c + DAT_8005ee38 * 2;
        *puVar3 = *puVar4;
        DAT_8005ee38 = iVar8;
        puVar3[1] = puVar4[1];
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
      puVar3[1] = puVar3[1] + puVar4[iVar7 + 1];
      puVar3 = puVar3 + -2;
      iVar8 = iVar5;
    } while (-1 < iVar5);
  }
  return;
code_r0x800463c0:
  in_v1 = (uint *)(param_3 * 8 + in_t0);
  goto code_r0x800463c8;
}

