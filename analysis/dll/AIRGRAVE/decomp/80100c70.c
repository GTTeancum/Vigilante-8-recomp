// addr: 0x80100c70  name: FUN_80100c70

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100c70(uint *param_1,undefined4 param_2,int *param_3)

{
  short sVar1;
  char cVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  uint *puVar6;
  uint uVar7;
  uint uVar8;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  int local_28;
  undefined4 local_24;
  int local_20;
  
  switch(param_2) {
  case 0:
    if (-1 < (char)param_1[2]) {
      param_1[0x12] = param_1[0x12] + param_1[0x20];
      param_1[0x13] = param_1[0x13] + param_1[0x21];
      param_1[0x14] = param_1[0x14] + param_1[0x22];
      param_1[9] = param_1[0x12];
      param_1[10] = param_1[0x13];
      param_1[0xb] = param_1[0x14];
    }
    switch((int)(((byte)param_1[2] + 1) * 0x1000000) >> 0x18) {
    case 0:
      FUN_8001787c/*0x8001787c*/(param_1,param_1[0x17] + 4);
      if ((_DAT_80065310 - (uint)*(byte *)((int)param_1 + 9) & 3) != 0) {
        return 0;
      }
      puVar6 = (uint *)FUN_8001ac44/*0x8001ac44*/(_DAT_800737d8,0x21,0x80,8);
      sVar1 = (short)_DAT_80065310;
      *puVar6 = *puVar6 | 0x4b4;
      uVar8 = param_1[10];
      uVar7 = param_1[0xb];
      puVar6[0x12] = param_1[9];
      puVar6[0x13] = uVar8;
      puVar6[0x14] = uVar7;
      *(short *)(puVar6 + 0x11) = sVar1 * 0x60;
      puVar6[0x19] = (uint)FUN_80100c18;
      FUN_8002036c/*0x8002036c*/();
    case 1:
      uVar8 = param_1[0x20];
      param_1[0x20] = uVar8 + 0x20;
      if (0x3b9a < (int)(uVar8 + 0x20)) {
        *(undefined1 *)(param_1 + 2) = 1;
switchD_80100d54_caseD_2:
        *(short *)(param_1 + 0x11) = (short)param_1[0x11] + 1;
        *(short *)((int)param_1 + 0x42) = *(short *)((int)param_1 + 0x42) + 1;
        FUN_8001d708/*0x8001d708*/(param_1);
        if (-0x7bf < (int)param_1[0x21]) {
          param_1[0x21] = param_1[0x21] - 6;
        }
        iVar3 = (short)param_1[5] * 0x3b9a;
        if (iVar3 < 0) {
          iVar3 = iVar3 + 0xfff;
        }
        param_1[0x20] = iVar3 >> 0xc;
        iVar3 = (short)param_1[8] * 0x3b9a;
        if (iVar3 < 0) {
          iVar3 = iVar3 + 0xfff;
        }
        param_1[0x22] = iVar3 >> 0xc;
        if (0x155 < (short)param_1[0x11]) {
          *(undefined1 *)(param_1 + 2) = 2;
switchD_80100d54_caseD_3:
          if (_DAT_80065b38 + 0x400000U < param_1[0x12]) {
            *(undefined2 *)(param_1 + 0x10) = 0;
            *(undefined2 *)((int)param_1 + 0x42) = 0xfc00;
            *(undefined2 *)(param_1 + 0x11) = 0;
            param_1[0x20] = 0xffffc466;
            param_1[0x21] = 0x7bf;
            param_1[0x22] = 0;
            param_1[0x12] = 0x4d30968;
            iVar3 = Terrain_HeightAt/*0x80025400*/(0x4018000,0x3c50000);
            param_1[0x14] = 0x3c50000;
            param_1[0x13] = iVar3 + param_1[0x21] * -900;
            *(undefined1 *)(param_1 + 2) = 3;
            FUN_8001d708/*0x8001d708*/(param_1);
switchD_80100d54_caseD_4:
            iVar3 = Terrain_HeightAt/*0x80025400*/(param_1[0x12],param_1[0x14]);
            if (iVar3 <= (int)param_1[0x13]) {
              param_1[0x21] = 0;
              *(undefined1 *)(param_1 + 2) = 4;
switchD_80100d54_caseD_5:
              uVar8 = param_1[0x20];
              param_1[0x20] = uVar8 + 0x18;
              if (-0xbec < (int)(uVar8 + 0x18)) {
                *(undefined1 *)(param_1 + 2) = 5;
switchD_80100d54_caseD_6:
                if ((int)param_1[0x12] < 0x3a44b29) {
                  *(undefined1 *)(param_1 + 2) = 6;
switchD_80100d54_caseD_7:
                  *(short *)((int)param_1 + 0x42) = *(short *)((int)param_1 + 0x42) + -8;
                  FUN_8001d708/*0x8001d708*/(param_1);
                  iVar3 = (short)param_1[5] * 0xbeb;
                  if (iVar3 < 0) {
                    iVar3 = iVar3 + 0xfff;
                  }
                  param_1[0x20] = iVar3 >> 0xc;
                  iVar3 = (short)param_1[8] * 0xbeb;
                  if (iVar3 < 0) {
                    iVar3 = iVar3 + 0xfff;
                  }
                  param_1[0x22] = iVar3 >> 0xc;
                  if (*(short *)((int)param_1 + 0x42) < -0x7ff) {
                    *(undefined1 *)(param_1 + 2) = 7;
switchD_80100d54_caseD_8:
                    if ((int)param_1[0x14] < 0x39acb29) {
                      *(undefined1 *)(param_1 + 2) = 8;
switchD_80100d54_caseD_9:
                      *(short *)((int)param_1 + 0x42) = *(short *)((int)param_1 + 0x42) + -8;
                      FUN_8001d708/*0x8001d708*/(param_1);
                      iVar3 = (short)param_1[5] * 0xbeb;
                      if (iVar3 < 0) {
                        iVar3 = iVar3 + 0xfff;
                      }
                      param_1[0x20] = iVar3 >> 0xc;
                      iVar3 = (short)param_1[8] * 0xbeb;
                      if (iVar3 < 0) {
                        iVar3 = iVar3 + 0xfff;
                      }
                      param_1[0x22] = iVar3 >> 0xc;
                      if (*(short *)((int)param_1 + 0x42) < -0xbff) {
                        *(undefined1 *)(param_1 + 2) = 0;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    default:
      if (param_3 == (int *)0x0) {
        return 0;
      }
      for (uVar8 = param_1[0xe]; uVar8 != 0; uVar8 = *(uint *)(uVar8 + 0x34)) {
        if (*(short *)(uVar8 + 6) == 0) {
          *(short *)(uVar8 + 0x44) = *(short *)(uVar8 + 0x44) + (short)param_3 * 0x100;
          FUN_8001d708/*0x8001d708*/(uVar8);
        }
      }
      iVar3 = FUN_800449bc/*0x800449bc*/(param_1 + 0x12);
      FUN_80044574/*0x80044574*/((int)*(char *)((int)param_1 + 5),iVar3 << 1);
      break;
    case 2:
      goto switchD_80100d54_caseD_2;
    case 3:
      goto switchD_80100d54_caseD_3;
    case 4:
      goto switchD_80100d54_caseD_4;
    case 5:
      goto switchD_80100d54_caseD_5;
    case 6:
      goto switchD_80100d54_caseD_6;
    case 7:
      goto switchD_80100d54_caseD_7;
    case 8:
      goto switchD_80100d54_caseD_8;
    case 9:
      goto switchD_80100d54_caseD_9;
    }
    break;
  case 1:
    goto switchD_80100cb4_caseD_1;
  case 2:
    goto switchD_80100cb4_caseD_2;
  case 3:
    break;
  case 4:
    goto switchD_80100cb4_caseD_4;
  default:
    goto switchD_80100cb4_caseD_5;
  case 7:
    goto switchD_80100cb4_caseD_7;
  case 8:
    goto switchD_80100cb4_caseD_8;
  }
  iVar3 = *param_3;
  if (*(short *)(param_3[1] + 2) == 0) {
LAB_801012c8:
    cVar2 = *(char *)(iVar3 + 4);
  }
  else {
    cVar2 = *(char *)(iVar3 + 4);
    if (cVar2 == '\x02') {
      FUN_8001f5a0/*0x8001f5a0*/(param_1,param_3);
      local_38 = 0x80000;
      if (param_3[5] < 0) {
        local_38 = 0xfff80000;
      }
      local_34 = 0xfffc0000;
      local_30 = 0x40000;
      GTE_RotateLongMat/*0x80043358*/(param_1 + 4,&local_38,&local_38);
      param_3 = param_3 + 5;
      GTE_RotateLongMatTrans/*0x80043408*/(param_1 + 4,param_3,param_3);
      FUN_800176f8/*0x800176f8*/(iVar3,&local_38,param_3);
      FUN_800435c0/*0x800435c0*/(iVar3 + 0x10,param_3,param_3);
      uVar5 = FUN_8004410c/*0x8004410c*/();
      FUN_800447e8/*0x800447e8*/(uVar5,*(undefined4 *)(param_1[0x16] + 8),3,param_3);
      FUN_80040234/*0x80040234*/(param_3);
      FUN_8002c6fc/*0x8002c6fc*/(iVar3,0xffffff9c,param_3,1);
      goto LAB_801012c8;
    }
  }
  if (cVar2 == '\a') {
    param_3 = (int *)(uint)*(ushort *)(iVar3 + 0xc);
switchD_80100cb4_caseD_8:
    iVar3 = FUN_80022320/*0x80022320*/(param_1,param_3);
    if (iVar3 != 0) {
      iVar3 = 0;
      *(undefined1 *)(param_1 + 2) = 0xff;
      param_1[0x20] = param_1[0x20] << 7;
      param_1[0x21] = param_1[0x21] << 7;
      param_1[0x22] = param_1[0x22] << 7;
      FUN_80020890/*0x80020890*/(param_1,300);
      do {
        iVar3 = iVar3 + 1;
        iVar4 = FUN_80017160/*0x80017160*/();
        local_28 = (iVar4 * 0xbeb >> 0xf) + -0x5f5;
        local_24 = 0xffffee1f;
        iVar4 = FUN_80017160/*0x80017160*/();
        local_20 = (iVar4 * 0xbeb >> 0xf) + -0x5f5;
        FUN_8003cee0/*0x8003cee0*/(0x7f780000,param_1 + 0x12,&local_28);
      } while (iVar3 < 3);
      FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
      *(undefined1 *)((int)param_1 + 5) = 0;
switchD_80100cb4_caseD_2:
      FUN_8003fc50/*0x8003fc50*/(param_1);
      FUN_800205f8/*0x800205f8*/(param_1);
switchD_80100cb4_caseD_7:
      FUN_8001ac44/*0x8001ac44*/(param_1,(uint)param_3 & 0xffff,0xa8,0);
switchD_80100cb4_caseD_1:
      *(undefined2 *)(param_1 + 0x27) = 0x40;
      *(undefined2 *)((int)param_1 + 0x9e) = 0x40;
      *(undefined2 *)(param_1 + 0x28) = 0x40;
      puVar6 = param_1;
      if ((char)param_1[2] != '\0') {
        param_1[0x19] = (uint)FUN_8010068c;
        *(undefined1 *)(param_1 + 2) = 0;
        *param_1 = *param_1 | 0x12a;
        FUN_80020890/*0x80020890*/(param_1,0x3c);
        puVar6 = (uint *)0x1;
      }
      *param_1 = *param_1 | 0x188;
      cVar2 = FUN_8004410c/*0x8004410c*/(puVar6);
      *(char *)((int)param_1 + 5) = cVar2;
      FUN_800443c8/*0x800443c8*/((int)cVar2,*(undefined4 *)(param_1[0x16] + 8),1,0);
      uVar5 = FUN_8001bda0/*0x8001bda0*/(param_1[0x16],0xe0);
      func_0x8003e564(param_1,uVar5);
switchD_80100cb4_caseD_4:
      FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
    }
switchD_80100cb4_caseD_5:
  }
  return 0;
}

