// addr: 0x80100eb4  name: FUN_80100eb4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100eb4(undefined4 *param_1,int param_2,int *param_3)

{
  bool bVar1;
  char cVar2;
  uint uVar3;
  undefined4 *puVar4;
  undefined4 uVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  uint uVar9;
  
  uVar9 = 1;
  puVar4 = param_1;
  if (param_2 != 1) {
    if (param_2 == 0) {
LAB_80100ef8:
      uVar6 = (uint)*(char *)(param_1 + 2);
      if (uVar6 == 1) {
LAB_80101004:
        uVar3 = param_1[9] - 0x1dcd;
        param_1[9] = uVar3;
        if ((int)uVar3 < 0x33e0000) {
LAB_80101028:
          iVar7 = param_1[9];
          param_1[9] = iVar7 + 0x1dcd;
          if ((int)uVar3 < iVar7 + 0x1dcd) {
            FUN_800205f8/*0x800205f8*/(param_1);
          }
        }
      }
      else {
        uVar3 = 2;
        if (((1 < (int)uVar6) || (uVar3 = 0x4d30000, uVar6 != 0)) &&
           (bVar1 = uVar6 == uVar3, uVar3 = 0x3920000, bVar1)) goto LAB_80101028;
        iVar8 = param_1[0xb];
        iVar7 = iVar8 + -0x1dcd;
        param_1[0xb] = iVar7;
        if (((iVar7 <= (int)(uVar3 | 0xffff)) && (0x4d40000 < iVar8)) ||
           ((iVar7 < 0x4b50000 && (0x4b50000 < iVar8)))) {
          param_1[0x12] = param_1[9];
          param_1[0x13] = param_1[10];
          param_1[0x14] = param_1[0xb];
          iVar7 = func_0x80021c6c(param_1);
          *(undefined1 *)(iVar7 + 8) = 1;
          *(undefined2 *)(iVar7 + 0x42) = 0x400;
          FUN_8002036c/*0x8002036c*/();
          iVar7 = func_0x80021c6c(param_1);
          *(undefined1 *)(iVar7 + 8) = 2;
          *(undefined2 *)(iVar7 + 0x42) = 0xfc00;
          FUN_8002036c/*0x8002036c*/();
        }
        if ((int)param_1[0xb] < 0x4a90000) {
          DAT_80101308 = DAT_80101308 + -1;
          goto LAB_80101004;
        }
      }
      if ((_DAT_80065310 - (uint)*(byte *)((int)param_1 + 9) & 3) == 0) {
        uVar9 = 0x80;
        puVar4 = (undefined4 *)FUN_8001ac44/*0x8001ac44*/(_DAT_800737d8,0x1f,0x80,8);
        *puVar4 = 0x10;
        uVar6 = FUN_80017160/*0x80017160*/();
        puVar4[9] = ((uVar6 & 0xff) - 0x80) * 0x400;
        puVar4[10] = 0;
        puVar4[0xb] = 0xfffec000;
        puVar4[0x19] = FUN_80100e70;
        FUN_8001d4f0/*0x8001d4f0*/(param_1);
      }
      if (param_3 == (int *)0x0) {
        return 0;
      }
      uVar5 = FUN_800446dc/*0x800446dc*/(param_1 + 9);
      FUN_80044574/*0x80044574*/((int)*(char *)((int)param_1 + 5),uVar5);
    }
    else if (param_2 != 3) {
      if (param_2 == 4) goto LAB_801011f8;
      goto LAB_80100ef8;
    }
    iVar7 = *param_3;
    if (*(char *)(iVar7 + 4) != '\x02') {
      return 0;
    }
    uVar6 = (uint)*(char *)(param_1 + 2);
    if (uVar6 == uVar9) {
LAB_8010115c:
      iVar8 = 1;
      uVar6 = 0xfffecf00;
    }
    else {
      iVar8 = iVar7;
      if ((int)uVar6 < 2) {
        bVar1 = uVar6 != 0;
        uVar6 = 0xfffe0000;
        if (bVar1) {
          iVar8 = 1;
          goto LAB_80101138;
        }
LAB_80101148:
        *(uint *)(iVar7 + 0x88) = *(int *)(iVar7 + 0x88) + (uVar6 | 0xcf00);
        goto LAB_8010115c;
      }
LAB_80101138:
      bVar1 = uVar6 != 2;
      uVar6 = 0x10000;
      if (bVar1) goto LAB_80101148;
    }
    *(uint *)(iVar7 + 0x80) = *(int *)(iVar7 + 0x80) + (uVar6 | 0x3100);
    uVar9 = FUN_80017160/*0x80017160*/(iVar8);
    if ((uVar9 & 0x1f) == 0) {
      FUN_8002c4bc/*0x8002c4bc*/(iVar7);
    }
    uVar5 = FUN_8004410c/*0x8004410c*/();
    FUN_8004483c/*0x8004483c*/(uVar5,*(undefined4 *)(param_1[0x16] + 8),5,param_1 + 9);
    puVar4 = (undefined4 *)0x1;
  }
  *param_1 = 0x84;
  cVar2 = FUN_8004410c/*0x8004410c*/(puVar4);
  *(char *)((int)param_1 + 5) = cVar2;
  FUN_800443c8/*0x800443c8*/((int)cVar2,*(undefined4 *)(param_1[0x16] + 8),3,0);
LAB_801011f8:
  FUN_800441c8/*0x800441c8*/((int)*(char *)((int)param_1 + 5));
  return 0;
}

