// addr: 0x8002efe0  name: FUN_8002efe0

void FUN_8002efe0(uint *param_1,short *param_2)

{
  short sVar1;
  short sVar2;
  uint uVar3;
  int iVar4;
  undefined1 uVar5;
  ushort uVar6;
  uint uVar7;
  int iVar8;
  int iVar9;
  uint *puVar10;
  uint uVar11;
  uint uVar12;
  
  sVar1 = *param_2;
  uVar11 = *(uint *)(param_2 + 4);
  puVar10 = param_1 + 0x29;
  if (sVar1 == 3) {
    *(ushort *)(param_1 + 0x29) = (*(byte *)(param_2 + 8) - 0x80) * 5;
    uVar12 = *(byte *)((int)param_2 + 0x11) < 0x81 ^ 1;
    if (-1 < *(char *)((int)param_2 + 0x15)) {
      uVar12 = uVar12 | 0x10000;
    }
    if ((*param_1 & 0x8000000) == 0) {
      if ((*(char *)((int)param_1 + 0xb2) < '\0') && (0x10 < *(byte *)((int)param_2 + 0x11))) {
        uVar5 = 1;
LAB_8002f4f4:
        *(undefined1 *)((int)param_1 + 0xb2) = uVar5;
      }
      else {
        uVar5 = 0xff;
        if ((uVar11 & 0x100) != 0) goto LAB_8002f4f4;
      }
      uVar6 = (ushort)param_1[0x2b];
      if (*(char *)((int)param_1 + 0xb2) < '\0') {
        if ((uVar11 & 0x100) == 0) {
          uVar6 = 0;
        }
      }
      else {
        uVar3 = (uint)*(byte *)((int)param_2 + 0x11);
        if (*(byte *)(param_2 + 9) < 0xf1) {
LAB_8002f54c:
          uVar3 = uVar3 - *(byte *)(param_2 + 9);
        }
        else {
          iVar4 = (int)(short)*puVar10;
          if (iVar4 < 0) {
            iVar4 = -iVar4;
          }
          if (iVar4 < 0xaa) goto LAB_8002f54c;
        }
        iVar4 = uVar3 * uVar6;
        uVar6 = (ushort)((uint)iVar4 >> 8);
        if (iVar4 < 0) {
          uVar6 = (ushort)((uint)(iVar4 + 0xff) >> 8);
        }
      }
      *(ushort *)((int)param_1 + 0xa6) = uVar6;
      FUN_8002ee94(param_1,uVar12);
    }
    else {
      FUN_8002c59c(param_1,uVar12);
    }
    if ((short)param_1[6] < 0) {
LAB_8002f88c:
      param_1[0x26] =
           param_1[0x26] + ((uint)*(byte *)(param_2 + 8) - (uint)*(byte *)(param_2 + 10)) * 0x100;
      return;
    }
    if ((*(byte *)(param_2 + 9) < 0xf1) && ((uVar11 & 0x400) == 0)) {
      iVar4 = param_1[0x23] * (int)*(short *)((int)param_1 + 0xaa);
      if (iVar4 < 0) {
        iVar4 = iVar4 + 0xfff;
      }
      iVar4 = (int)(short)param_1[0x2a] + (iVar4 >> 0xc);
      iVar8 = 0;
      if (0 < iVar4) {
        iVar8 = iVar4;
      }
      uVar11 = param_1[0x25];
      iVar4 = (short)*puVar10 * iVar8;
      if (*(char *)((int)param_1 + 0xb2) < '\0') {
        iVar4 = (int)(short)*puVar10 * -iVar8;
      }
      goto LAB_8002f958;
    }
    iVar4 = (int)(short)*puVar10;
    if (iVar4 < 0) {
      iVar4 = -iVar4;
    }
    if (iVar4 < 0xaa) {
      *(short *)((int)param_1 + 0xa6) = (short)param_1[0x2b] * -2;
    }
    uVar11 = param_1[0x25];
    iVar4 = (int)(short)*puVar10 << 1;
    if (*(char *)((int)param_1 + 0xb2) < '\0') goto LAB_8002f5f4;
  }
  else {
    if (sVar1 < 4) {
      if (sVar1 != 2) {
        return;
      }
      uVar12 = 0x100;
      if (*(char *)((int)param_1 + 0xb2) < '\0') {
        uVar12 = 0x200;
        uVar3 = 0x100;
      }
      else {
        uVar3 = 0x200;
      }
      if ((uVar11 & uVar3) == 0) {
        uVar3 = uVar11 & (uVar12 | uVar12 << 0x10);
        if ((*param_1 & 0x8000000) == 0) {
          uVar7 = uVar3;
          if ((*(uint *)(param_2 + 6) & 0xf0000000) != 0) {
            uVar7 = uVar3 & 0xffff;
          }
          FUN_8002ee94(param_1,uVar7);
          if ((uVar3 & 0xffff) == 0) {
            sVar1 = *(short *)((int)param_1 + 0xa6) + -2;
            if (-1 < *(short *)((int)param_1 + 0xa6)) goto LAB_8002f130;
            *(undefined2 *)((int)param_1 + 0xa6) = 0;
          }
          else {
            *(short *)((int)param_1 + 0xa6) = (short)param_1[0x2b];
          }
        }
        else {
          FUN_8002c59c(param_1,uVar3);
        }
      }
      else if ((int)param_1[0x23] < 0x1da) {
        uVar5 = 1;
        if (-1 < *(char *)((int)param_1 + 0xb2)) {
          uVar5 = 0xff;
        }
        *(undefined1 *)((int)param_1 + 0xb2) = uVar5;
      }
      else {
        sVar1 = -(short)param_1[0x2b];
LAB_8002f130:
        *(short *)((int)param_1 + 0xa6) = sVar1;
      }
      if ((uVar11 & 0x1800) == 0) {
        iVar4 = (int)(short)*puVar10 * param_1[0x23];
        if (iVar4 < 0) {
          iVar4 = iVar4 + 0x7fff;
        }
        *(short *)puVar10 = (short)*puVar10 - (short)(iVar4 >> 0xf);
        if ((uVar11 & 0x400) == 0) {
          return;
        }
        *(short *)((int)param_1 + 0xa6) = (short)param_1[0x2b] * -2;
        return;
      }
      if ((short)param_1[6] < 1) {
        if ((uVar11 & 0x8000000) != 0) {
          param_1[0x26] = param_1[0x26] - 0x4000;
          return;
        }
        if ((uVar11 & 0x10000000) == 0) {
          return;
        }
        param_1[0x26] = param_1[0x26] + 0x4000;
        return;
      }
      uVar3 = 0;
      if (*(char *)((int)param_1 + 0xb2) < '\x01') {
        if ((uVar11 & 0x18000000) != 0) {
          uVar3 = *param_1 & 0xbfffffff;
          if (*(char *)((int)param_1 + 0xb2) < '\0') {
            if ((uVar11 & uVar12) == 0) {
              uVar12 = param_1[0x20];
              if ((int)uVar12 < 0) {
                uVar12 = uVar12 + 0x7f;
              }
              uVar7 = param_1[0x22];
              if ((int)uVar7 < 0) {
                uVar7 = uVar7 + 0x7f;
              }
              if (-0x4c4001 <
                  (int)(short)param_1[5] * ((int)uVar12 >> 7) +
                  (int)(short)param_1[8] * ((int)uVar7 >> 7)) goto LAB_8002f200;
            }
            uVar3 = uVar3 | 0x40000000;
          }
LAB_8002f200:
          *param_1 = uVar3;
        }
        uVar3 = *param_1 >> 0x1e & 1;
      }
      else {
        *param_1 = *param_1 & 0xbfffffff;
      }
      if ((uVar11 & 0x400) == 0) {
        if ((uVar11 & 0x800) == 0) {
          iVar8 = (int)(short)*puVar10;
          iVar4 = 0;
          if (0 < iVar8) {
            iVar4 = iVar8;
          }
          if (iVar4 < 0) {
            iVar4 = iVar4 + 0x3f;
          }
          iVar9 = (iVar8 + 0x10) - (iVar4 >> 6);
          iVar8 = 0x2aa0000;
          iVar4 = 0x2aa;
          if (iVar9 < 0x2aa) {
            iVar8 = iVar9 * 0x10000;
            iVar4 = iVar9;
          }
          *(short *)puVar10 = (short)iVar4;
          if (iVar8 >> 0x10 < 1) {
            return;
          }
          iVar9 = param_1[0x23] * (int)*(short *)((int)param_1 + 0xaa);
          iVar4 = (uVar3 * 2 + -1) * (iVar8 >> 0x10);
          sVar1 = (short)param_1[0x2a];
        }
        else {
          iVar8 = (int)(short)*puVar10;
          iVar4 = 0;
          if (iVar8 < 0) {
            iVar4 = iVar8;
          }
          if (iVar4 < 0) {
            iVar4 = iVar4 + 0x3f;
          }
          iVar9 = (iVar8 + -0x10) - (iVar4 >> 6);
          iVar8 = -0x2aa0000;
          iVar4 = -0x2aa;
          if (-0x2aa < iVar9) {
            iVar8 = iVar9 * 0x10000;
            iVar4 = iVar9;
          }
          *(short *)puVar10 = (short)iVar4;
          if (-1 < iVar8 >> 0x10) {
            return;
          }
          iVar9 = param_1[0x23] * (int)*(short *)((int)param_1 + 0xaa);
          iVar4 = (uVar3 * 2 + -1) * (iVar8 >> 0x10);
          sVar1 = (short)param_1[0x2a];
        }
        if (iVar9 < 0) {
          iVar9 = iVar9 + 0xfff;
        }
        iVar9 = (int)sVar1 + (iVar9 >> 0xc);
        iVar8 = 0;
        if (0 < iVar9) {
          iVar8 = iVar9;
        }
        iVar4 = iVar4 * iVar8;
        if (iVar4 < 0) {
          iVar4 = iVar4 + 0xf;
        }
        uVar11 = param_1[0x25] - (iVar4 >> 4);
        goto LAB_8002f970;
      }
      if ((uVar11 & 0x800) == 0) {
        iVar8 = (short)*puVar10 + 0x20;
        iVar4 = 0x2aa;
        if (iVar8 < 0x2aa) {
          iVar4 = iVar8;
        }
        *(short *)puVar10 = (short)iVar4;
        uVar11 = param_1[0x25];
        if (uVar3 != 0) goto LAB_8002f288;
      }
      else {
        iVar8 = (short)*puVar10 + -0x20;
        iVar4 = -0x2aa;
        if (-0x2aa < iVar8) {
          iVar4 = iVar8;
        }
        *(short *)puVar10 = (short)iVar4;
        uVar11 = param_1[0x25];
        if (uVar3 == 0) {
LAB_8002f288:
          uVar11 = uVar11 - 0x500;
          goto LAB_8002f970;
        }
      }
      uVar11 = uVar11 + 0x500;
      goto LAB_8002f970;
    }
    if (5 < sVar1) {
      return;
    }
    iVar8 = *(byte *)(param_2 + 9) - 0x80;
    iVar4 = iVar8;
    if (iVar8 < 0) {
      iVar4 = -iVar8;
    }
    if (0x20 < iVar4) {
      if (iVar8 < 0) {
        iVar8 = *(byte *)(param_2 + 9) - 0x7d;
      }
      *(short *)(param_1[0x38] + 0x8e) = *(short *)(param_1[0x38] + 0x8e) - (short)(iVar8 >> 2);
    }
    iVar8 = *(byte *)((int)param_2 + 0x13) - 0x80;
    iVar4 = iVar8;
    if (iVar8 < 0) {
      iVar4 = -iVar8;
    }
    if (0x20 < iVar4) {
      iVar8 = iVar8 * 0xbeb;
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0x7f;
      }
      iVar8 = *(int *)(param_1[0x38] + 0x94) + (iVar8 >> 7);
      iVar4 = param_1[0x15] << 1;
      if (((int)(param_1[0x15] << 1) <= iVar8) && (iVar4 = 0x140000, iVar8 < 0x140001)) {
        iVar4 = iVar8;
      }
      *(int *)(param_1[0x38] + 0x94) = iVar4;
    }
    iVar8 = *(byte *)(param_2 + 8) - 0x80;
    iVar4 = iVar8;
    if (iVar8 < 0) {
      iVar4 = -iVar8;
    }
    *(short *)puVar10 = (short)((iVar4 * iVar8) / 0x18);
    uVar12 = 0;
    if (*(byte *)((int)param_2 + 0x11) < 0x41) {
      uVar12 = *(byte *)((int)param_2 + 0x15) < 0x41 ^ 1;
    }
    uVar3 = uVar11 & 0x1000100;
    if (*(byte *)((int)param_2 + 0x11) < 0x40) {
      uVar3 = uVar3 | 1;
    }
    uVar3 = uVar12 << 0x10 | uVar3;
    if ((*param_1 & 0x8000000) == 0) {
      sVar1 = -1;
      if (-1 < *(char *)((int)param_1 + 0xb2)) {
        sVar1 = 1;
      }
      if ((uVar11 & 0x100) == 0) {
        iVar4 = (0x80 - (uint)*(byte *)((int)param_2 + 0x11)) * (uint)(ushort)param_1[0x2b];
        if (iVar4 < 0) {
          iVar4 = iVar4 + 0x7f;
        }
        sVar2 = (short)(iVar4 >> 7);
      }
      else {
        sVar2 = (short)param_1[0x2b];
      }
      *(short *)((int)param_1 + 0xa6) = sVar1 * sVar2;
      if (((int)param_1[0x23] < 0x1da) && (*(short *)((int)param_1 + 0xa6) < -0x10)) {
        uVar5 = 1;
        if (-1 < *(char *)((int)param_1 + 0xb2)) {
          uVar5 = 0xff;
        }
        *(undefined1 *)((int)param_1 + 0xb2) = uVar5;
      }
      FUN_8002ee94(param_1,uVar3);
    }
    else {
      FUN_8002c59c(param_1,uVar3);
    }
    if ((short)param_1[6] < 0) goto LAB_8002f88c;
    if ((uVar11 & 0x400) != 0) {
      iVar4 = (int)(short)*puVar10;
      if (iVar4 < 0) {
        iVar4 = -iVar4;
      }
      if (iVar4 < 0xaa) {
        *(short *)((int)param_1 + 0xa6) = (short)param_1[0x2b] * -2;
      }
      uVar11 = param_1[0x25];
      iVar4 = (short)*puVar10 * 2;
      if (-1 < *(char *)((int)param_1 + 0xb2)) {
        uVar11 = uVar11 + iVar4;
        goto LAB_8002f970;
      }
LAB_8002f5f4:
      uVar11 = uVar11 - iVar4;
      goto LAB_8002f970;
    }
    iVar4 = param_1[0x23] * (int)*(short *)((int)param_1 + 0xaa);
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0xfff;
    }
    iVar4 = (int)(short)param_1[0x2a] + (iVar4 >> 0xc);
    iVar8 = 0;
    if (0 < iVar4) {
      iVar8 = iVar4;
    }
    uVar11 = param_1[0x25];
    iVar4 = (short)*puVar10 * iVar8;
    if (*(char *)((int)param_1 + 0xb2) < '\0') {
      iVar4 = (int)(short)*puVar10 * -iVar8;
    }
LAB_8002f958:
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0xf;
    }
    iVar4 = iVar4 >> 4;
  }
  uVar11 = uVar11 + iVar4;
LAB_8002f970:
  param_1[0x25] = uVar11;
  return;
}

