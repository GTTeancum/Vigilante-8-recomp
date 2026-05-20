// addr: 0x8004d914  name: RotMatrixY

MATRIX * RotMatrixY(long r,MATRIX *m)

{
  int iVar1;
  MATRIX *pMVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  
  if (-1 < r) {
    iVar1 = -(int)(short)*(int *)(&DAT_800607b4 + (r & 0xfffU) * 2);
    iVar3 = *(int *)(&DAT_800607b4 + (r & 0xfffU) * 2) >> 0x10;
    iVar4 = (int)m->m[0][0];
    iVar7 = (int)m->m[2][0];
    iVar5 = (int)m->m[0][1];
    iVar8 = (int)m->m[2][1];
    iVar6 = (int)m->m[0][2];
    iVar9 = (int)m->m[2][2];
    m->m[0][0] = (short)(iVar3 * iVar4 - iVar1 * iVar7 >> 0xc);
    m->m[0][1] = (short)(iVar3 * iVar5 - iVar1 * iVar8 >> 0xc);
    m->m[0][2] = (short)(iVar3 * iVar6 - iVar1 * iVar9 >> 0xc);
    m->m[2][0] = (short)(iVar1 * iVar4 + iVar3 * iVar7 >> 0xc);
    m->m[2][1] = (short)(iVar1 * iVar5 + iVar3 * iVar8 >> 0xc);
    m->m[2][2] = (short)(iVar1 * iVar6 + iVar3 * iVar9 >> 0xc);
    return m;
  }
  pMVar2 = (MATRIX *)FGO_05_OBJ_64();
  return pMVar2;
}

