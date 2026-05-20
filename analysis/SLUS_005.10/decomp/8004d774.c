// addr: 0x8004d774  name: RotMatrixX

MATRIX * RotMatrixX(long r,MATRIX *m)

{
  MATRIX *pMVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  
  if (-1 < r) {
    iVar3 = (int)(short)*(int *)(&DAT_800607b4 + (r & 0xfffU) * 2);
    iVar2 = *(int *)(&DAT_800607b4 + (r & 0xfffU) * 2) >> 0x10;
    iVar4 = (int)m->m[1][0];
    iVar7 = (int)m->m[2][0];
    iVar5 = (int)m->m[1][1];
    iVar8 = (int)m->m[2][1];
    iVar6 = (int)m->m[1][2];
    iVar9 = (int)m->m[2][2];
    m->m[1][0] = (short)(iVar2 * iVar4 - iVar3 * iVar7 >> 0xc);
    m->m[1][1] = (short)(iVar2 * iVar5 - iVar3 * iVar8 >> 0xc);
    m->m[1][2] = (short)(iVar2 * iVar6 - iVar3 * iVar9 >> 0xc);
    m->m[2][0] = (short)(iVar3 * iVar4 + iVar2 * iVar7 >> 0xc);
    m->m[2][1] = (short)(iVar3 * iVar5 + iVar2 * iVar8 >> 0xc);
    m->m[2][2] = (short)(iVar3 * iVar6 + iVar2 * iVar9 >> 0xc);
    return m;
  }
  pMVar1 = (MATRIX *)FGO_04_OBJ_64();
  return pMVar1;
}

