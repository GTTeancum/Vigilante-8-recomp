// addr: 0x8001a640  name: FUN_8001a640

undefined4 * FUN_8001a640(int *param_1,undefined4 param_2)

{
  byte bVar1;
  undefined4 *puVar2;
  int iVar3;
  int *piVar4;
  uint uVar5;
  int iVar6;
  undefined4 *puVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  
  puVar2 = (undefined4 *)FUN_800116f4(param_1[4] * 0xc + 0xc);
  *puVar2 = param_1;
  puVar2[1] = param_2;
  puVar2[2] = 0;
  iVar9 = 0;
  param_1[1] = (int)param_1 + param_1[1];
  if (0 < *param_1) {
    do {
      piVar4 = (int *)(iVar9 * 4 + param_1[1]);
      iVar6 = param_1[1] + *piVar4;
      *piVar4 = iVar6;
      iVar10 = 0;
      iVar3 = iVar6 + *(int *)(iVar6 + 0x14);
      *(int *)(iVar6 + 4) = iVar6 + *(int *)(iVar6 + 4);
      *(int *)(iVar6 + 0xc) = iVar6 + *(int *)(iVar6 + 0xc);
      *(int *)(iVar6 + 0x14) = iVar3;
      if (*(short *)(iVar6 + 0x10) != 0) {
        do {
          bVar1 = *(byte *)(iVar3 + 3);
          uVar5 = (bVar1 & 0xf) << 2;
          if ((bVar1 & 0x80) != 0) {
            uVar5 = uVar5 | 0x40;
          }
          if ((bVar1 & 0x10) != 0) {
            uVar5 = uVar5 | 2;
          }
          if ((bVar1 & 0x40) != 0) {
            uVar5 = uVar5 | 0x80;
          }
          *(char *)(iVar3 + 3) = (char)uVar5;
          *(short *)(iVar3 + 4) = *(short *)(iVar3 + 4) << 3;
          *(short *)(iVar3 + 6) = *(short *)(iVar3 + 6) << 3;
          *(short *)(iVar3 + 8) = *(short *)(iVar3 + 8) << 3;
          iVar8 = iVar3;
          switch(uVar5 >> 2 & 0xf) {
          case 1:
          case 3:
                    /* Possible PsyQ macro: setPolyGT3() */
            *(undefined1 *)(iVar3 + 0x1b) = 0x34;
                    /* Possible PsyQ macro: setPolyGT3() */
            *(undefined1 *)(iVar3 + 0x17) = 0x34;
            break;
          case 2:
                    /* Possible PsyQ macro: setPolyG3() */
            *(undefined1 *)(iVar3 + 0x13) = 0x30;
                    /* Possible PsyQ macro: setPolyG3() */
            *(undefined1 *)(iVar3 + 0xf) = 0x30;
            break;
          case 4:
          case 5:
          case 7:
            *(short *)(iVar3 + 10) = *(short *)(iVar3 + 10) << 3;
            break;
          case 8:
          case 9:
          case 0xb:
          case 0xc:
            *(short *)(iVar3 + 10) = *(short *)(iVar3 + 10) << 3;
            *(short *)(iVar3 + 0xc) = *(short *)(iVar3 + 0xc) << 3;
            *(short *)(iVar3 + 0xe) = *(short *)(iVar3 + 0xe) << 3;
            break;
          case 10:
            iVar8 = iVar3 + (uint)*(ushort *)(iVar3 + 10) * 4;
            break;
          case 0xd:
          case 0xf:
            *(byte *)(iVar3 + 3) = *(byte *)(iVar3 + 3) | 1;
          }
          iVar10 = iVar10 + 1;
          iVar3 = iVar8 + (uint)*(ushort *)(&DAT_800568fc + (*(byte *)(iVar3 + 3) & 0x3c));
        } while (iVar10 < (int)(uint)*(ushort *)(iVar6 + 0x10));
      }
      iVar9 = iVar9 + 1;
    } while (iVar9 < *param_1);
  }
  iVar9 = 0;
  param_1[3] = (int)param_1 + param_1[3];
  if (0 < param_1[2]) {
    do {
      piVar4 = (int *)(iVar9 * 4 + param_1[3]);
      *piVar4 = param_1[3] + *piVar4;
      iVar9 = iVar9 + 1;
    } while (iVar9 < param_1[2]);
  }
  iVar9 = 0;
  param_1[5] = (int)param_1 + param_1[5];
  puVar7 = puVar2;
  if (0 < param_1[4]) {
    do {
      piVar4 = (int *)(iVar9 * 4 + param_1[5]);
      *piVar4 = param_1[5] + *piVar4;
      *(undefined2 *)(puVar7 + 3) = 0;
      iVar9 = iVar9 + 1;
      puVar7 = puVar7 + 3;
    } while (iVar9 < param_1[4]);
  }
  return puVar2;
}

