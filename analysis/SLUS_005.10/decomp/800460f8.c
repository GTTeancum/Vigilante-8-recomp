// addr: 0x800460f8  name: S_M_M_OBJ_54

uint S_M_M_OBJ_54(void)

{
  uint uVar1;
  uint *puVar2;
  uint *puVar3;
  uint uVar4;
  uint uVar5;
  int unaff_s0;
  uint unaff_s1;
  uint uVar6;
  int unaff_s2;
  int unaff_s3;
  
  if ((unaff_s1 & ~DAT_8005ee04) != 0) {
    unaff_s1 = unaff_s1 + DAT_8005ee04;
  }
  uVar6 = ((int)unaff_s1 >> (DAT_8005edfc & 0x1f)) << (DAT_8005edfc & 0x1f);
  if ((*DAT_8005ee3c & 0x40000000) != 0) {
    uVar6 = S_M_M_OBJ_12C();
    return uVar6;
  }
  _spu_gcSPU();
  if (unaff_s0 < DAT_8005ee34) {
    puVar3 = DAT_8005ee3c + unaff_s0 * 2;
    do {
      if (((*puVar3 & 0x40000000) != 0) || (((*puVar3 & 0x80000000) != 0 && (uVar6 <= puVar3[1]))))
      {
        uVar6 = S_M_M_OBJ_12C();
        return uVar6;
      }
      unaff_s0 = unaff_s0 + 1;
      puVar3 = puVar3 + 2;
    } while (unaff_s0 < DAT_8005ee34);
  }
  uVar1 = 0xffffffff;
  if (unaff_s2 != -1) {
    puVar3 = DAT_8005ee3c + unaff_s2 * 2;
    if ((*puVar3 & 0x40000000) == 0) {
      uVar1 = puVar3[1];
      if (uVar6 < uVar1) {
        if (DAT_8005ee38 < DAT_8005ee34) {
          puVar2 = DAT_8005ee3c + DAT_8005ee38 * 2;
          uVar5 = *puVar2;
          uVar4 = puVar2[1];
          *puVar2 = *puVar3 + uVar6 | 0x80000000;
          puVar2[1] = uVar1 - uVar6;
          DAT_8005ee38 = DAT_8005ee38 + 1;
          puVar2[2] = uVar5;
          puVar2[3] = uVar4;
        }
      }
      puVar3 = DAT_8005ee3c + unaff_s2 * 2;
      puVar3[1] = uVar6;
      *puVar3 = *puVar3 & 0xfffffff;
      _spu_gcSPU();
      uVar1 = DAT_8005ee3c[unaff_s2 * 2];
    }
    else {
      uVar1 = 0xffffffff;
      if ((unaff_s2 < DAT_8005ee34) && (uVar1 = 0xffffffff, uVar6 <= puVar3[1] - unaff_s3)) {
        DAT_8005ee38 = unaff_s2 + 1;
        puVar2 = DAT_8005ee3c + DAT_8005ee38 * 2;
        *puVar2 = (*puVar3 & 0xfffffff) + uVar6 | 0x40000000;
        puVar2[1] = puVar3[1] - uVar6;
        puVar3[1] = uVar6;
        *puVar3 = *puVar3 & 0xfffffff;
        _spu_gcSPU();
        uVar6 = S_M_M_OBJ_2A8();
        return uVar6;
      }
    }
  }
  return uVar1;
}

