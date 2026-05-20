// addr: 0x8002ea94  name: FUN_8002ea94

void FUN_8002ea94(uint *param_1)

{
  int *piVar1;
  int *piVar2;
  MATRIX *r0;
  int iVar3;
  uint uVar4;
  int iVar5;
  uint *puVar6;
  uint *puVar7;
  uint *puVar8;
  uint uVar9;
  uint *puVar10;
  uint *puVar11;
  int unaff_s6;
  int unaff_s7;
  undefined1 *puVar12;
  undefined1 *puVar13;
  undefined1 auStack_50 [32];
  uint *local_30;
  
  puVar12 = (undefined1 *)0x0;
  puVar13 = auStack_50;
  local_30 = param_1 + 0x29;
  if ((uRam00000010 == 0) || (*(short *)((int)param_1 + 6) != -1)) {
    r0 = (MATRIX *)&DAT_8006f680;
  }
  else {
    r0 = (MATRIX *)&DAT_8006f6a0;
  }
  puVar10 = (uint *)0x0;
  puVar7 = (uint *)0x0;
  uVar9 = 0xffffffff;
  gte_SetRotMatrix(r0);
  gte_SetTransMatrix(r0);
  piVar1 = *(int **)(puVar12 + 0x714);
  for (piVar2 = (int *)**(int **)(puVar12 + 0x714); piVar2 != (int *)0x0; piVar2 = (int *)*piVar2) {
    puVar6 = (uint *)piVar1[2];
    if ((((puVar6 != param_1) && ((char)puVar6[1] != '\x03')) && ((*puVar6 & 0x4000) != 0)) &&
       ((0 < *(short *)((int)puVar6 + 6) || (puVar12[0x15] == '\x03')))) {
      FUN_800432d0(puVar6 + 0x12,puVar13 + 0x10);
      iVar3 = *(int *)(puVar13 + 0x14) >> 10;
      if (iVar3 < 0) {
        iVar3 = -iVar3;
      }
      iVar5 = *(int *)(puVar13 + 0x10) >> 10;
      if (iVar5 < 0) {
        iVar5 = -iVar5;
      }
      if (iVar3 < iVar5) {
        iVar3 = iVar5;
      }
      iVar5 = *(int *)(puVar13 + 0x18) >> 10;
      if (((iVar3 < iVar5) &&
          ((puVar8 = puVar6, uVar4 = uVar9, puVar11 = puVar10, puVar7 == (uint *)0x0 ||
           (unaff_s6 * iVar3 < iVar5 * unaff_s7)))) ||
         ((puVar7 == (uint *)0x0 &&
          (uVar4 = FUN_80016aac(param_1 + 9,puVar6 + 0x12), puVar8 = puVar7, puVar11 = puVar6,
          iVar5 = unaff_s6, iVar3 = unaff_s7, uVar4 < uVar9)))) {
        puVar7 = puVar8;
        uVar9 = uVar4;
        puVar10 = puVar11;
        unaff_s6 = iVar5;
        unaff_s7 = iVar3;
      }
    }
    piVar1 = piVar2;
  }
  if (puVar7 == (uint *)0x0) {
    puVar7 = puVar10;
  }
  if ((puVar7 != (uint *)param_1[0x39]) &&
     ((puVar7 != (uint *)0x0 || (*(int *)(puVar13 + 0x54) != 0)))) {
    param_1[0x39] = (uint)puVar7;
    iVar3 = *(int *)(puVar13 + 0x20);
    *(undefined2 *)(iVar3 + 0x18) = 0;
    uVar9 = param_1[*(byte *)(iVar3 + 0xf) + 0x44];
    if ((uVar9 != 0) && (*(code **)(uVar9 + 100) != (code *)0x0)) {
      (**(code **)(uVar9 + 100))(uVar9,10,0);
    }
  }
  return;
}

