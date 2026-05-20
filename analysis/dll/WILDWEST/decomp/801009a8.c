// addr: 0x801009a8  name: FUN_801009a8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801009a8(uint *param_1,uint param_2,int *param_3)

{
  longlong lVar1;
  short sVar2;
  char cVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  undefined4 uVar7;
  uint uVar8;
  uint extraout_v1;
  uint *puVar9;
  undefined8 uVar10;
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
  undefined4 local_3c;
  int local_38;
  uint local_30;
  uint local_2c;
  uint local_28;
  undefined1 auStack_20 [16];
  
  if (6 < param_2) {
    return 0;
  }
  switch(param_2) {
  case 0:
    if ((((&switchD_801009e4::switchdataD_80100140)[param_2] != (undefined *)0x0) &&
        (param_1[0x29] != 0)) && ((*(ushort *)(param_1[0x29] + 0xc) & 0x100) != 0)) {
      uVar7 = FUN_8004410c/*0x8004410c*/(param_1);
      FUN_800447e8/*0x800447e8*/(uVar7,*(undefined4 *)(param_1[0x16] + 8),6,param_1 + 9);
    }
    if ((char)param_1[2] < '\0') {
      param_3 = (int *)&DAT_80060000;
      if ((_DAT_80065310 - (uint)*(byte *)((int)param_1 + 9) & 3) != 0) {
        return 0;
      }
      puVar9 = (uint *)FUN_8001ac44/*0x8001ac44*/(_DAT_800737d8,0x21,0x80,8);
      sVar2 = (short)_DAT_80065310;
      *puVar9 = *puVar9 | 0x4b4;
      uVar8 = param_1[10];
      uVar4 = param_1[0xb];
      puVar9[0x12] = param_1[9];
      puVar9[0x13] = uVar8;
      puVar9[0x14] = uVar4;
      *(short *)(puVar9 + 0x11) = sVar2 * 0x60;
      puVar9[0x19] = (uint)FUN_80100950;
      FUN_8002036c/*0x8002036c*/();
    }
    if (param_3 == (int *)0x0) {
      return 0;
    }
    uVar7 = FUN_800449bc/*0x800449bc*/(param_1 + 9);
    FUN_80044574/*0x80044574*/((int)*(char *)((int)param_1 + 5),uVar7);
    break;
  case 1:
    goto switchD_801009e4_caseD_1;
  case 2:
    goto switchD_801009e4_caseD_2;
  case 4:
    goto switchD_801009e4_caseD_4;
  case 5:
    goto switchD_801009e4_caseD_5;
  case 6:
    goto switchD_801009e4_caseD_6;
  }
  iVar6 = *param_3;
  uVar8 = (uint)*(byte *)(iVar6 + 4);
  uVar4 = 2;
  if (uVar8 == 7) {
    if ((char)param_1[2] < '\0') {
      return 0;
    }
    iVar5 = FUN_80022320/*0x80022320*/(param_1,*(undefined2 *)(iVar6 + 0xc));
    if (iVar5 == 0) {
      return 0;
    }
    iVar5 = (int)(short)param_1[5] * param_1[0x2a];
    param_1[0x29] = 0;
    *(undefined1 *)(param_1 + 2) = 0xff;
    *param_1 = *param_1 & 0xfffffeff;
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0x1f;
    }
    param_1[0x20] = iVar5 >> 5;
    param_1[0x21] = 0;
    iVar5 = (int)(short)param_1[8] * param_1[0x2a];
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0x1f;
    }
    param_1[0x22] = iVar5 >> 5;
    FUN_80020890/*0x80020890*/(param_1,300);
    param_1[0x25] = 30000;
    FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
    *(undefined1 *)((int)param_1 + 5) = 0;
    FUN_80020890/*0x80020890*/(param_1,300);
    FUN_8002002c/*0x8002002c*/(0x80065a18,9,param_1);
    uVar4 = 0;
    uVar8 = extraout_v1;
  }
  if (uVar8 == uVar4) {
    func_0x8001f974(param_1,param_3,auStack_80);
    FUN_800434f8/*0x800434f8*/(param_1 + 4,auStack_60,auStack_48);
    if (local_44 < 0x801) {
      return 0;
    }
    iVar5 = (int)(short)param_1[5] * param_1[0x2a];
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0x1f;
    }
    local_40 = *(int *)(iVar6 + 0x80) - (iVar5 >> 5);
    local_3c = *(undefined4 *)(iVar6 + 0x84);
    iVar5 = (int)(short)param_1[8] * param_1[0x2a];
    if (iVar5 < 0) {
      iVar5 = iVar5 + 0x1f;
    }
    local_38 = *(int *)(iVar6 + 0x88) - (iVar5 >> 5);
    uVar10 = FUN_80017240/*0x80017240*/(&local_40,auStack_60);
    param_1 = (uint *)((uint)uVar10 >> 0xb | (int)((ulonglong)uVar10 >> 0x20) << 0x15);
    if (-1 < (int)param_1) {
      return 0;
    }
    uVar4 = (int)((uint)local_58 << 0x10) >> 0x10;
    uVar8 = -((int)param_1 + local_50);
    iVar5 = -(uint)(uVar8 != 0) - ((int)param_1 + local_50 >> 0x1f);
    lVar1 = (ulonglong)uVar4 * (ulonglong)uVar8;
    local_30 = (uint)lVar1 >> 0xc |
               ((int)((ulonglong)lVar1 >> 0x20) + uVar4 * iVar5 +
               uVar8 * ((int)((uint)local_58 << 0x10) >> 0x1f)) * 0x100000;
    uVar4 = (int)((uint)local_56 << 0x10) >> 0x10;
    lVar1 = (ulonglong)uVar4 * (ulonglong)uVar8;
    local_2c = (uint)lVar1 >> 0xc |
               ((int)((ulonglong)lVar1 >> 0x20) + uVar4 * iVar5 +
               uVar8 * ((int)((uint)local_56 << 0x10) >> 0x1f)) * 0x100000;
    uVar4 = (int)((uint)local_54 << 0x10) >> 0x10;
    lVar1 = (ulonglong)uVar4 * (ulonglong)uVar8;
    local_28 = (uint)lVar1 >> 0xc |
               ((int)((ulonglong)lVar1 >> 0x20) + uVar4 * iVar5 +
               uVar8 * ((int)((uint)local_54 << 0x10) >> 0x1f)) * 0x100000;
    FUN_80017594/*0x80017594*/(iVar6,&local_30,auStack_6c);
    FUN_8002c958/*0x8002c958*/(iVar6,(int)((int)param_1 + 0x1fffU) >> 0xd,auStack_6c,1);
    GTE_RotateLongMatTrans/*0x80043408*/(iVar6 + 0x10,auStack_6c,auStack_20);
    uVar7 = FUN_8004410c/*0x8004410c*/();
    FUN_800447e8/*0x800447e8*/(uVar7,_DAT_800658fc,5,auStack_20);
    FUN_80040234/*0x80040234*/(auStack_20);
  }
  if (*(code **)(iVar6 + 100) != (code *)0x0) {
    (**(code **)(iVar6 + 100))(iVar6,8,1000);
switchD_801009e4_caseD_2:
    puVar9 = param_1;
    if ((char)param_1[2] < '\0') {
      FUN_8003fc50/*0x8003fc50*/(param_1);
      FUN_800205f8/*0x800205f8*/(param_1);
      puVar9 = (uint *)0x1;
    }
    *(undefined1 *)(param_1 + 2) = 0xff;
    FUN_8003fc50/*0x8003fc50*/(puVar9);
    FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
    *(undefined1 *)((int)param_1 + 5) = 0;
    FUN_80020890/*0x80020890*/(param_1,300);
switchD_801009e4_caseD_6:
switchD_801009e4_caseD_1:
    iVar6 = FUN_8001b038/*0x8001b038*/(param_1,0x8000);
    if (iVar6 != 0) {
      uVar7 = FUN_800407b4/*0x800407b4*/(_DAT_800737d8,6,&DAT_80100130);
      FUN_8001b2fc/*0x8001b2fc*/(param_1,iVar6,uVar7);
      FUN_80020744/*0x80020744*/(uVar7);
      if ((*param_1 & 4) == 0) {
        FUN_800207c4/*0x800207c4*/(uVar7);
      }
      cVar3 = FUN_8004410c/*0x8004410c*/();
      *(char *)((int)param_1 + 5) = cVar3;
      FUN_800443c8/*0x800443c8*/((int)cVar3,*(undefined4 *)(param_1[0x16] + 8),0,0);
switchD_801009e4_caseD_4:
      FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
    }
  }
switchD_801009e4_caseD_5:
  return 0;
}

