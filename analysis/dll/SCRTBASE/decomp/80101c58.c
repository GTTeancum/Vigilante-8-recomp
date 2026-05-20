// addr: 0x80101c58  name: FUN_80101c58

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80101c58(uint *param_1,undefined4 param_2,int *param_3)

{
  longlong lVar1;
  int *piVar2;
  char cVar3;
  uint uVar4;
  int *piVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  int iVar8;
  uint *puVar9;
  uint uVar10;
  int *piVar11;
  int iVar12;
  undefined8 uVar13;
  undefined1 auStack_80 [20];
  undefined1 auStack_6c [12];
  undefined1 auStack_60 [8];
  ushort local_58;
  ushort local_56;
  ushort local_54;
  int local_50;
  undefined1 auStack_48 [4];
  short local_44;
  int local_40;
  int local_3c;
  int local_38;
  uint local_30;
  uint local_2c;
  uint local_28;
  int aiStack_20 [4];
  
  puVar9 = param_1;
  switch(param_2) {
  case 0:
    param_1[0x12] = param_1[0x12] + param_1[0x20];
    param_1[0x13] = param_1[0x13] + param_1[0x21];
    param_1[0x14] = param_1[0x14] + param_1[0x22];
    param_1[9] = param_1[0x12];
    param_1[10] = param_1[0x13];
    param_1[0xb] = param_1[0x14];
    switch((int)(((byte)param_1[2] - 1) * 0x1000000) >> 0x18) {
    case 0:
      uVar4 = Terrain_HeightAt/*0x80025400*/(param_1[0x12],param_1[0x14]);
      param_1[0x13] = uVar4;
      uVar4 = 0xbeb;
      if ((int)(param_1[0x23] + 0x65) < 0xbeb) {
        uVar4 = param_1[0x23] + 0x65;
      }
      iVar8 = (int)(short)param_1[5] * uVar4;
      param_1[0x23] = uVar4;
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0xfff;
      }
      iVar12 = (int)(short)param_1[8] * param_1[0x23];
      param_1[0x20] = iVar8 >> 0xc;
      if (iVar12 < 0) {
        iVar12 = iVar12 + 0xfff;
      }
      param_1[0x22] = iVar12 >> 0xc;
      if ((int)param_1[0x12] < 0x4810000) {
        *(undefined1 *)(param_1 + 2) = 2;
        goto switchD_80101d24_caseD_1;
      }
      break;
    case 1:
switchD_80101d24_caseD_1:
      *(short *)((int)param_1 + 0x42) = *(short *)((int)param_1 + 0x42) + -8;
      FUN_8001d708/*0x8001d708*/(param_1);
      iVar8 = (short)param_1[5] * 0xbeb;
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0xfff;
      }
      param_1[0x20] = iVar8 >> 0xc;
      iVar8 = (short)param_1[8] * 0xbeb;
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0xfff;
      }
      param_1[0x22] = iVar8 >> 0xc;
      if (*(short *)((int)param_1 + 0x42) < 0x801) {
        *(undefined1 *)(param_1 + 2) = 3;
        goto switchD_80101d24_caseD_2;
      }
      break;
    case 2:
switchD_80101d24_caseD_2:
      if ((int)param_1[0x14] < 0x4f5cb29) {
        *(undefined1 *)(param_1 + 2) = 4;
        goto switchD_80101d24_caseD_3;
      }
      break;
    case 3:
switchD_80101d24_caseD_3:
      *(short *)((int)param_1 + 0x42) = *(short *)((int)param_1 + 0x42) + 8;
      FUN_8001d708/*0x8001d708*/(param_1);
      iVar8 = (short)param_1[5] * 0xbeb;
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0xfff;
      }
      param_1[0x20] = iVar8 >> 0xc;
      iVar8 = (short)param_1[8] * 0xbeb;
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0xfff;
      }
      param_1[0x22] = iVar8 >> 0xc;
      if (0xbff < *(short *)((int)param_1 + 0x42)) {
        *(undefined1 *)(param_1 + 2) = 5;
        goto switchD_80101d24_caseD_4;
      }
      break;
    case 4:
switchD_80101d24_caseD_4:
      if ((int)param_1[0x12] < 0x4360000) {
        *(undefined1 *)(param_1 + 2) = 6;
        goto switchD_80101d24_caseD_5;
      }
      break;
    case 5:
switchD_80101d24_caseD_5:
      iVar8 = (int)(short)param_1[5] * (param_1[0x23] - 0x65);
      param_1[0x23] = param_1[0x23] - 0x65;
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0xfff;
      }
      param_1[0x20] = iVar8 >> 0xc;
      if ((int)param_1[0x23] < 0) {
        *(undefined1 *)(param_1 + 2) = 7;
        goto switchD_80101d24_caseD_6;
      }
      break;
    case 6:
switchD_80101d24_caseD_6:
      *(short *)((int)param_1 + 0x42) = *(short *)((int)param_1 + 0x42) + -8;
      FUN_8001d708/*0x8001d708*/(param_1);
      if (*(short *)((int)param_1 + 0x42) < 0x401) {
        FUN_80020890/*0x80020890*/(param_1,0xb4);
        *(undefined1 *)(param_1 + 2) = 8;
        goto switchD_80101d24_caseD_8;
      }
      break;
    case 8:
switchD_80101d24_caseD_8:
      if ((int)param_1[0x20] < 0x5968) {
        param_1[0x20] = param_1[0x20] + 0x40;
      }
      if (0x46a0000 < (int)param_1[0x12]) {
        *(undefined1 *)(param_1 + 2) = 10;
        goto switchD_80101d24_caseD_9;
      }
      break;
    case 9:
switchD_80101d24_caseD_9:
      *(short *)(param_1 + 0x11) = (short)param_1[0x11] + 1;
      *(short *)((int)param_1 + 0x42) = *(short *)((int)param_1 + 0x42) + 1;
      if ((short)param_1[0x10] < 0xe3) {
        *(short *)(param_1 + 0x10) = (short)param_1[0x10] + 1;
      }
      FUN_8001d708/*0x8001d708*/(param_1);
      iVar8 = (short)param_1[5] * 0x5968;
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0xfff;
      }
      param_1[0x20] = iVar8 >> 0xc;
      iVar8 = *(short *)((int)param_1 + 0x1a) * 0x5968;
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0xfff;
      }
      param_1[0x21] = iVar8 >> 0xc;
      iVar8 = (short)param_1[8] * 0x5968;
      if (iVar8 < 0) {
        iVar8 = iVar8 + 0xfff;
      }
      param_1[0x22] = iVar8 >> 0xc;
      if (0x155 < (short)param_1[0x11]) {
        FUN_8004445c/*0x8004445c*/((int)*(char *)((int)param_1 + 5),_DAT_800658fc,0x37);
        FUN_800205f8/*0x800205f8*/(param_1);
        DAT_80102bcc = 0;
        FUN_8002185c/*0x8002185c*/(9,1000);
      }
    }
    if (param_3 == (int *)0x0) {
      return 0;
    }
    uVar6 = FUN_800446dc/*0x800446dc*/(param_1 + 0x12);
    FUN_80044574/*0x80044574*/((int)*(char *)((int)param_1 + 5),uVar6);
    puVar9 = (uint *)0x1;
  case 2:
    if ((char)param_1[2] == '\b') {
      uVar6 = FUN_8004410c/*0x8004410c*/(puVar9);
      FUN_8004445c/*0x8004445c*/(uVar6,_DAT_800658fc,0x37);
      FUN_8003fea8/*0x8003fea8*/(param_1 + 0x12,0x8ff0000);
      uVar6 = FUN_8001d564/*0x8001d564*/(param_1[0xe]);
      FUN_800204dc/*0x800204dc*/(uVar6);
      param_3 = (int *)FUN_8001b038/*0x8001b038*/(param_1,0x8000);
      uVar6 = FUN_8001ac44/*0x8001ac44*/(param_1[0x16],0x68,0x80,8);
      FUN_8001b2fc/*0x8001b2fc*/(param_1,param_3,uVar6);
      *(undefined1 *)(param_1 + 2) = 9;
switchD_80101c94_caseD_3:
      iVar8 = *param_3;
      if (*(char *)(iVar8 + 4) == '\x02') {
        puVar9 = param_1;
        if ((uint *)param_3[3] != param_1) {
          FUN_8002c3ac/*0x8002c3ac*/(iVar8);
          puVar9 = (uint *)0x1;
        }
        func_0x8001f974(puVar9,param_3,auStack_80);
        FUN_800434f8/*0x800434f8*/(param_1 + 4,auStack_60,auStack_48);
        if (0x800 < local_44) {
          local_40 = *(int *)(iVar8 + 0x80) + param_1[0x20] * -0x80;
          local_3c = *(int *)(iVar8 + 0x84) + param_1[0x21] * -0x80;
          local_38 = *(int *)(iVar8 + 0x88) + param_1[0x22] * -0x80;
          uVar13 = FUN_80017240/*0x80017240*/(&local_40,auStack_60);
          param_1 = (uint *)((uint)uVar13 >> 0xb | (int)((ulonglong)uVar13 >> 0x20) << 0x15);
          if ((int)param_1 < 0) {
            uVar10 = (int)((uint)local_58 << 0x10) >> 0x10;
            uVar4 = -((int)param_1 + local_50);
            iVar12 = -(uint)(uVar4 != 0) - ((int)param_1 + local_50 >> 0x1f);
            lVar1 = (ulonglong)uVar10 * (ulonglong)uVar4;
            local_30 = (uint)lVar1 >> 0xc |
                       ((int)((ulonglong)lVar1 >> 0x20) + uVar10 * iVar12 +
                       uVar4 * ((int)((uint)local_58 << 0x10) >> 0x1f)) * 0x100000;
            uVar10 = (int)((uint)local_56 << 0x10) >> 0x10;
            lVar1 = (ulonglong)uVar10 * (ulonglong)uVar4;
            local_2c = (uint)lVar1 >> 0xc |
                       ((int)((ulonglong)lVar1 >> 0x20) + uVar10 * iVar12 +
                       uVar4 * ((int)((uint)local_56 << 0x10) >> 0x1f)) * 0x100000;
            uVar10 = (int)((uint)local_54 << 0x10) >> 0x10;
            lVar1 = (ulonglong)uVar10 * (ulonglong)uVar4;
            local_28 = (uint)lVar1 >> 0xc |
                       ((int)((ulonglong)lVar1 >> 0x20) + uVar10 * iVar12 +
                       uVar4 * ((int)((uint)local_54 << 0x10) >> 0x1f)) * 0x100000;
            FUN_80017594/*0x80017594*/(iVar8,&local_30,auStack_6c);
            FUN_8002c958/*0x8002c958*/(iVar8,(int)((int)param_1 + 0x1fffU) >> 0xd,auStack_6c,1);
            param_3 = aiStack_20;
            GTE_RotateLongMatTrans/*0x80043408*/(iVar8 + 0x10,auStack_6c,param_3);
            uVar6 = FUN_8004410c/*0x8004410c*/();
            FUN_800447e8/*0x800447e8*/(uVar6,_DAT_800658fc,5,param_3);
            FUN_80040234/*0x80040234*/(param_3);
switchD_80101c94_caseD_9:
            if ((param_3 == (int *)(int)*(short *)((int)param_1 + 6)) ||
               ((param_3 == (int *)0x3e8 && ((*param_1 & 0x10000) != 0)))) {
              if (DAT_80102bcc != 0) {
                *param_1 = *param_1 | 0x10000;
              }
              piVar5 = (int *)func_0x800217e0(param_1);
              piVar11 = (int *)piVar5[1];
              iVar8 = *piVar5;
              *(int **)(iVar8 + 4) = piVar11;
              *piVar11 = iVar8;
              piVar11 = _DAT_80065a20;
              piVar2 = piVar5;
              *_DAT_80065a20 = (int)piVar5;
              _DAT_80065a20 = piVar2;
              piVar5[1] = (int)piVar11;
              *piVar5 = -0x7ff9a5e4;
              cVar3 = FUN_8004410c/*0x8004410c*/();
              *(char *)((int)param_1 + 5) = cVar3;
              FUN_800443c8/*0x800443c8*/((int)cVar3,*(undefined4 *)(param_1[0x16] + 8),4,0);
              iVar8 = FUN_8001fd9c/*0x8001fd9c*/((int)*(short *)((int)param_1 + 6));
              FUN_80024718/*0x80024718*/(iVar8 + 0xc,0x8f80);
              *(undefined1 *)(param_1 + 2) = 1;
              *param_1 = *param_1 | 0x100;
              uVar6 = FUN_8001b038/*0x8001b038*/(param_1,0x8000);
              uVar7 = FUN_8001ac44/*0x8001ac44*/(param_1[0x16],0x66,0x80,8);
              FUN_8001b2fc/*0x8001b2fc*/(param_1,uVar6,uVar7);
              *param_1 = *param_1 | 0x800;
              FUN_80020744/*0x80020744*/(param_1);
              FUN_800207c4/*0x800207c4*/(param_1);
              DAT_80102bcc = 1;
switchD_80101c94_caseD_6:
              goto switchD_80101c94_caseD_1;
            }
          }
        }
      }
    }
    break;
  case 1:
switchD_80101c94_caseD_1:
    *(undefined2 *)(param_1 + 0x27) = 0x40;
    *(undefined2 *)((int)param_1 + 0x9e) = 0x40;
    *(undefined2 *)(param_1 + 0x28) = 0x40;
    *param_1 = *param_1 | 8;
    uVar6 = FUN_8001bda0/*0x8001bda0*/(param_1[0x16],0x41);
    func_0x8003e564(param_1,uVar6);
  case 4:
    FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
  default:
    break;
  case 3:
    goto switchD_80101c94_caseD_3;
  case 6:
    goto switchD_80101c94_caseD_6;
  case 9:
    goto switchD_80101c94_caseD_9;
  }
  return 0;
}

