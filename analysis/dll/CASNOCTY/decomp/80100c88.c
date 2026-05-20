// addr: 0x80100c88  name: FUN_80100c88

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100c88(uint *param_1,undefined4 param_2,uint *param_3)

{
  char cVar1;
  short sVar2;
  uint *puVar3;
  int iVar4;
  undefined4 uVar5;
  int iVar6;
  int iVar7;
  int extraout_v1;
  uint uVar8;
  uint uVar9;
  uint *puVar10;
  undefined8 uVar11;
  
  switch(param_2) {
  case 0:
    param_1[0x12] = param_1[0x12] + param_1[0x20];
    param_1[0x13] = param_1[0x13] + param_1[0x21];
    param_1[0x14] = param_1[0x14] + param_1[0x22];
    switch((int)(((byte)param_1[2] + 1) * 0x1000000) >> 0x18) {
    case 0:
      goto switchD_80100d3c_caseD_0;
    case 1:
      iVar6 = FUN_80017160/*0x80017160*/();
      uVar8 = FUN_8001ffd4/*0x8001ffd4*/(0x80065a50,((iVar6 << 3) >> 0xf) + -0x20);
      param_1[0x23] = uVar8;
      uVar5 = FUN_80024d30/*0x80024d30*/(param_1 + 9,uVar8 + 0x48,0x113a0,0);
      FUN_80042e78/*0x80042e78*/(param_1 + 0x25,uVar5);
      *(undefined1 *)(param_1 + 2) = 1;
      break;
    case 2:
      break;
    case 3:
      goto switchD_80100d3c_caseD_3;
    case 4:
      goto switchD_80100d3c_caseD_4;
    case 5:
      goto switchD_80100d3c_caseD_5;
    case 6:
      goto switchD_80100d3c_caseD_6;
    default:
      goto switchD_80100d3c_default;
    }
    if ((_DAT_80065310 - (uint)*(byte *)((int)param_1 + 9) & 0xff) == 0) {
      uVar5 = FUN_80024d30/*0x80024d30*/(param_1 + 9,param_1[0x23] + 0x48,0x113a0,0);
      FUN_80042e78/*0x80042e78*/(param_1 + 0x25,uVar5);
    }
    if ((short)param_1[0x25] == 0) {
      param_1[0x27] = *(uint *)(param_1[0x23] + 0x48);
      param_1[0x28] = *(uint *)(param_1[0x23] + 0x50);
    }
    iVar6 = FUN_80042f98/*0x80042f98*/(param_1,param_1 + 0x25,0x40000);
    if ((short)param_1[0x25] < 0) {
      *(undefined1 *)(param_1 + 2) = 2;
    }
    iVar4 = (int)((iVar6 - (uint)(ushort)param_1[0x2a]) * 0x100000) >> 0x14;
    iVar6 = -8;
    if ((-9 < iVar4) && (iVar6 = 8, iVar4 < 9)) {
      iVar6 = iVar4;
    }
    iVar6 = (uint)(ushort)param_1[0x2a] + iVar6;
    iVar4 = iVar6 * 0x10000 >> 0x10;
    *(short *)(param_1 + 0x2a) = (short)iVar6;
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0x3f;
    }
    *(short *)((int)param_1 + 0x42) = *(short *)((int)param_1 + 0x42) + (short)(iVar4 >> 6);
switchD_80100d3c_caseD_0:
    if (((char)param_1[2] < '\0') &&
       ((((-1 < (int)param_1[0x20] || (*(short *)((int)param_1 + 0x4a) < *(short *)param_1[0x29]))
         && ((int)*(short *)param_1[0x29] + (int)((short *)param_1[0x29])[2] <
             (int)*(short *)((int)param_1 + 0x4a))) ||
        (((-1 < (int)param_1[0x22] ||
          (*(short *)((int)param_1 + 0x52) < *(short *)(param_1[0x29] + 2))) &&
         ((int)*(short *)(param_1[0x29] + 2) + (int)*(short *)(param_1[0x29] + 6) <
          (int)*(short *)((int)param_1 + 0x52))))))) {
      *(short *)((int)param_1 + 0x42) = *(short *)((int)param_1 + 0x42) + 0x10;
      param_1[0x20] = -param_1[0x20];
      param_1[0x22] = -param_1[0x22];
    }
    FUN_8001d708/*0x8001d708*/(param_1);
    iVar6 = (short)param_1[5] * 0xbeb;
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0xfff;
    }
    iVar6 = (iVar6 >> 0xc) - param_1[0x20];
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0xf;
    }
    iVar6 = iVar6 >> 4;
    iVar4 = -0x40;
    if ((-0x41 < iVar6) && (iVar4 = 0x40, iVar6 < 0x41)) {
      iVar4 = iVar6;
    }
    iVar6 = (short)param_1[8] * 0xbeb;
    param_1[0x20] = param_1[0x20] + iVar4;
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0xfff;
    }
    iVar6 = (iVar6 >> 0xc) - param_1[0x22];
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0xf;
    }
    iVar6 = iVar6 >> 4;
    iVar4 = -0x40;
    if ((-0x41 < iVar6) && (iVar4 = 0x40, iVar6 < 0x41)) {
      iVar4 = iVar6;
    }
    param_1[0x22] = param_1[0x22] + iVar4;
    iVar6 = Terrain_HeightAt/*0x80025400*/(param_1[0x12],param_1[0x14]);
    iVar6 = iVar6 - (param_1[0x13] + 0x64000);
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0xf;
    }
    iVar4 = -0x2fa;
    if (-0x2fa < iVar6 >> 4) {
      iVar4 = iVar6 >> 4;
    }
    iVar4 = iVar4 - param_1[0x21];
    iVar6 = -0x40;
    if ((-0x41 < iVar4) && (iVar6 = 0x40, iVar4 < 0x41)) {
      iVar6 = iVar4;
    }
    param_1[0x21] = param_1[0x21] + iVar6;
switchD_80100d3c_caseD_3:
    iVar4 = Terrain_HeightAt/*0x80025400*/(param_1[0x12],param_1[0x14]);
    iVar4 = iVar4 - (param_1[0x13] + 0x5000);
    iVar7 = ((int)(((uint)*(ushort *)(param_1[0x23] + 0x42) - (uint)*(ushort *)((int)param_1 + 0x42)
                   ) * 0x100000) >> 0x14) - (int)(short)param_1[0x2a];
    iVar6 = -8;
    if ((-9 < iVar7) && (iVar6 = 8, iVar7 < 9)) {
      iVar6 = iVar7;
    }
    iVar6 = (uint)(ushort)param_1[0x2a] + iVar6;
    iVar7 = iVar6 * 0x10000 >> 0x10;
    *(short *)(param_1 + 0x2a) = (short)iVar6;
    if (iVar7 < 0) {
      iVar7 = iVar7 + 0x3f;
    }
    iVar6 = -param_1[0x20];
    *(short *)((int)param_1 + 0x42) = *(short *)((int)param_1 + 0x42) + (short)(iVar7 >> 6);
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0xf;
    }
    iVar6 = iVar6 >> 4;
    iVar7 = -0x40;
    if ((-0x41 < iVar6) && (iVar7 = 0x40, iVar6 < 0x41)) {
      iVar7 = iVar6;
    }
    iVar6 = -param_1[0x22];
    param_1[0x20] = param_1[0x20] + iVar7;
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0xf;
    }
    iVar6 = iVar6 >> 4;
    iVar7 = -0x40;
    if ((-0x41 < iVar6) && (iVar7 = 0x40, iVar6 < 0x41)) {
      iVar7 = iVar6;
    }
    param_1[0x22] = param_1[0x22] + iVar7;
    iVar6 = iVar4;
    if (iVar4 < 0) {
      iVar6 = iVar4 + 0xf;
    }
    iVar7 = 0x2fa;
    if (iVar6 >> 4 < 0x2fa) {
      iVar7 = iVar6 >> 4;
    }
    iVar7 = iVar7 - param_1[0x21];
    iVar6 = -0x40;
    if ((-0x41 < iVar7) && (iVar6 = 0x40, iVar7 < 0x41)) {
      iVar6 = iVar7;
    }
    param_1[0x21] = param_1[0x21] + iVar6;
    FUN_8001d708/*0x8001d708*/(param_1);
    if ((iVar4 < 0x199) && ((short)param_1[0x2a] < 0x40)) {
      *(undefined1 *)(param_1 + 2) = 3;
      *(undefined2 *)((int)param_1 + 0xaa) = 900;
      uVar5 = FUN_8004410c/*0x8004410c*/();
      FUN_8004483c/*0x8004483c*/(uVar5,_DAT_800658fc,0x14,param_1 + 0x12);
switchD_80100d3c_caseD_4:
      sVar2 = *(short *)((int)param_1 + 0xaa) + -1;
      *(short *)((int)param_1 + 0xaa) = sVar2;
      if (sVar2 == -1) {
        *(undefined2 *)((int)param_1 + 0xaa) = 0;
        *(undefined1 *)(param_1 + 2) = 0;
switchD_80100d3c_caseD_5:
        if ((int)param_1[0x20] < 0) {
          if (*(short *)((int)param_1 + 0x4a) < *(short *)param_1[0x29]) {
            param_1[0x22] = 0;
            goto LAB_80101330;
          }
LAB_80101354:
          if ((int)param_1[0x22] < 0) {
            if (*(short *)((int)param_1 + 0x52) < *(short *)(param_1[0x29] + 2)) {
              param_1[0x22] = 0;
              goto LAB_80101388;
            }
          }
          else {
LAB_80101388:
            if ((int)*(short *)(param_1[0x29] + 2) + (int)*(short *)(param_1[0x29] + 6) <
                (int)*(short *)((int)param_1 + 0x52)) goto LAB_801013ac;
          }
        }
        else {
LAB_80101330:
          if ((int)*(short *)((int)param_1 + 0x4a) <=
              (int)*(short *)param_1[0x29] + (int)((short *)param_1[0x29])[2]) goto LAB_80101354;
LAB_801013ac:
          param_1[0x22] = 0;
          param_1[0x20] = 0;
        }
        if (-0x200 < (short)param_1[0x10]) {
          *(short *)(param_1 + 0x10) = (short)param_1[0x10] + -4;
        }
        FUN_8001d708/*0x8001d708*/(param_1);
        iVar6 = (short)param_1[5] * 0xbeb;
        if (iVar6 < 0) {
          iVar6 = iVar6 + 0xfff;
        }
        iVar6 = (iVar6 >> 0xc) - param_1[0x20];
        if (iVar6 < 0) {
          iVar6 = iVar6 + 0xf;
        }
        iVar6 = iVar6 >> 4;
        iVar4 = -0x40;
        if ((-0x41 < iVar6) && (iVar4 = 0x40, iVar6 < 0x41)) {
          iVar4 = iVar6;
        }
        iVar6 = *(short *)((int)param_1 + 0x1a) * 0xbeb;
        param_1[0x20] = param_1[0x20] + iVar4;
        if (iVar6 < 0) {
          iVar6 = iVar6 + 0xfff;
        }
        iVar6 = (iVar6 >> 0xc) - param_1[0x21];
        if (iVar6 < 0) {
          iVar6 = iVar6 + 0xf;
        }
        iVar6 = iVar6 >> 4;
        iVar4 = -0x40;
        if ((-0x41 < iVar6) && (iVar4 = 0x40, iVar6 < 0x41)) {
          iVar4 = iVar6;
        }
        iVar6 = (short)param_1[8] * 0xbeb;
        param_1[0x21] = param_1[0x21] + iVar4;
        if (iVar6 < 0) {
          iVar6 = iVar6 + 0xfff;
        }
        iVar6 = (iVar6 >> 0xc) - param_1[0x22];
        if (iVar6 < 0) {
          iVar6 = iVar6 + 0xf;
        }
        iVar6 = iVar6 >> 4;
        iVar4 = -0x40;
        if ((-0x41 < iVar6) && (iVar4 = 0x40, iVar6 < 0x41)) {
          iVar4 = iVar6;
        }
        param_1[0x22] = param_1[0x22] + iVar4;
        iVar6 = Terrain_HeightAt/*0x80025400*/(param_1[0x12],param_1[0x14]);
        if ((iVar6 < (int)param_1[0x13]) && ((*param_1 & 0x8000) == 0)) {
          FUN_8003fc50/*0x8003fc50*/(param_1);
          *(undefined1 *)(param_1 + 2) = 5;
switchD_80100d3c_caseD_6:
          if ((*param_1 & 0x8000) == 0) {
            FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
            *(undefined1 *)((int)param_1 + 5) = 0;
            FUN_800205f8/*0x800205f8*/(param_1);
          }
        }
      }
    }
switchD_80100d3c_default:
    if (param_3 == (uint *)0x0) {
      return 0;
    }
    param_3 = (uint *)((int)param_3 * 0x100);
    for (uVar8 = param_1[0xe]; uVar8 != 0; uVar8 = *(uint *)(uVar8 + 0x34)) {
      iVar6 = 1;
      if (*(short *)(uVar8 + 6) == 0) {
        iVar6 = (uint)*(ushort *)(uVar8 + 0x44) + (int)param_3;
        *(short *)(uVar8 + 0x44) = (short)iVar6;
      }
      if (*(short *)(uVar8 + 6) == iVar6) {
        if ((char)param_1[2] == '\x03') {
          if (0x354 < *(short *)(uVar8 + 0x40)) goto LAB_80101648;
          *(short *)(uVar8 + 0x40) = *(short *)(uVar8 + 0x40) + 0x20;
        }
        if (*(short *)(uVar8 + 0x40) != 0) {
          *(short *)(uVar8 + 0x40) = *(short *)(uVar8 + 0x40) + -0x20;
          FUN_8001d708/*0x8001d708*/(uVar8);
        }
      }
LAB_80101648:
    }
    uVar5 = FUN_800449bc/*0x800449bc*/(param_1 + 0x12);
    FUN_80044574/*0x80044574*/((int)*(char *)((int)param_1 + 5),uVar5);
    break;
  case 1:
switchD_80100cc4_caseD_1:
    *param_1 = 0x84;
    iVar6 = FUN_8001fd9c/*0x8001fd9c*/(0);
    param_1[0x29] = iVar6 + 0xc;
    cVar1 = FUN_8004410c/*0x8004410c*/();
    *(char *)((int)param_1 + 5) = cVar1;
    FUN_800443c8/*0x800443c8*/((int)cVar1,*(undefined4 *)(param_1[0x16] + 8),4,0);
  case 4:
    FUN_80042f5c/*0x80042f5c*/(param_1 + 0x25);
    FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
switchD_80100cc4_caseD_5:
    return 0;
  case 2:
    break;
  case 3:
    goto switchD_80100cc4_caseD_3;
  default:
    goto switchD_80100cc4_caseD_5;
  case 6:
    goto switchD_80100cc4_caseD_6;
  }
  *param_1 = *param_1 & 0xffffffdf;
switchD_80100cc4_caseD_3:
  puVar10 = (uint *)*param_3;
  if (((char)puVar10[1] == '\x02') && (*(short *)((int)puVar10 + 6) < 0)) {
    iVar6 = (int)(char)param_1[2];
    if (iVar6 == 3) {
      iVar6 = Math_Atan2_Pos/*0x80016c88*/(puVar10 + 4);
      iVar6 = (int)((iVar6 - (uint)*(ushort *)((int)param_1 + 0x42)) * 0x100000) >> 0x10;
      if (iVar6 < 0) {
        iVar6 = -iVar6;
      }
      if (iVar6 < 0x1000) {
        param_3 = (uint *)puVar10[0x38];
        param_1[0x24] = (uint)puVar10;
        puVar10[0x19] = (uint)FUN_801006d4;
        puVar10[0x1e] = (uint)param_1;
        *puVar10 = *puVar10 & 0xfffffff7 | 0x1000020;
        puVar10[0x20] = (param_1[0x12] - puVar10[9]) * 4;
        puVar10[0x21] = (param_1[0x13] - puVar10[10]) * 4;
        puVar10[0x22] = (param_1[0x14] - puVar10[0xb]) * 4;
        *(undefined2 *)(puVar10 + 0x29) = 0;
        FUN_80044574/*0x80044574*/((int)*(char *)((int)puVar10 + 5),0);
        FUN_80020890/*0x80020890*/(puVar10,0x20);
        if (param_3 != (uint *)0x0) {
          puVar3 = (uint *)Heap_AllocOrRetry/*0x800116f4*/(0x30);
          uVar8 = param_3[0x13];
          uVar9 = param_3[0x14];
          *puVar3 = param_3[0x12];
          puVar3[1] = uVar8;
          puVar3[2] = uVar9;
          puVar3[3] = 0x78;
          puVar3[4] = param_1[9] + (short)param_1[5] * -100;
          puVar3[5] = param_1[10] - 0x32000;
          puVar3[6] = param_1[0xb] + (short)param_1[8] * -100;
          puVar3[7] = 0;
          func_0x8003dbb0(param_3);
          param_3[0x25] = 0xfa000;
          *(undefined2 *)((int)param_1 + 0xaa) = 0;
          *(undefined1 *)(param_1 + 2) = 0;
        }
        FUN_8004410c/*0x8004410c*/();
      }
      goto LAB_8010185c;
    }
  }
  else {
LAB_8010185c:
    iVar6 = (int)(char)param_1[2];
  }
  if (iVar6 < 4) {
    uVar9 = (uint)(byte)puVar10[1];
    uVar8 = 3;
    if (uVar9 == 7) {
      uVar11 = FUN_80022320/*0x80022320*/(param_1,(short)puVar10[3]);
      uVar9 = (uint)((ulonglong)uVar11 >> 0x20);
      if ((int)uVar11 == 0) {
        return 0;
      }
      *(undefined1 *)(param_1 + 2) = 4;
      uVar8 = 0;
    }
    if (uVar9 == uVar8) {
      return 0;
    }
    FUN_8001f5a0/*0x8001f5a0*/(param_1,param_3);
    iVar6 = param_1[0x20] * (int)(short)param_3[8] +
            param_1[0x21] * (int)*(short *)((int)param_3 + 0x22) +
            param_1[0x22] * (int)(short)param_3[9];
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0x7ff;
    }
    iVar6 = iVar6 >> 0xb;
    if (-1 < iVar6) {
      return 0;
    }
    iVar4 = iVar6 * (short)param_3[8];
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0xfff;
    }
    param_1[0x20] = param_1[0x20] - (iVar4 >> 0xc);
    iVar4 = iVar6 * *(short *)((int)param_3 + 0x22);
    if (iVar4 < 0) {
      iVar4 = iVar4 + 0xfff;
    }
    param_1[0x21] = param_1[0x21] - (iVar4 >> 0xc);
    iVar6 = iVar6 * (short)param_3[9];
    if (iVar6 < 0) {
      iVar6 = iVar6 + 0xfff;
    }
    param_1[0x22] = param_1[0x22] - (iVar6 >> 0xc);
    uVar5 = FUN_8004410c/*0x8004410c*/();
    FUN_8004483c/*0x8004483c*/(uVar5,_DAT_800658fc,7,param_1 + 0x12);
    iVar6 = extraout_v1;
  }
  if (4 < iVar6) {
    return 0;
  }
  if ((char)puVar10[1] != '\0') {
    return 0;
  }
  if ((*param_1 & 0x8000) != 0) {
    return 0;
  }
  FUN_8003fc50/*0x8003fc50*/(param_1);
  *(undefined1 *)(param_1 + 2) = 5;
switchD_80100cc4_caseD_6:
  goto switchD_80100cc4_caseD_1;
}

