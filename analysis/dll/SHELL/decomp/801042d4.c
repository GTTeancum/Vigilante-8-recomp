// addr: 0x801042d4  name: FUN_801042d4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_801042d4(undefined4 param_1,uint param_2,int param_3,int param_4)

{
  int iVar1;
  undefined1 *puVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  uint *puVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  uint uVar9;
  int iVar10;
  undefined4 *puVar11;
  int iVar12;
  char *pcVar13;
  uint uVar14;
  uint local_res4;
  undefined1 auStack_60 [32];
  undefined4 local_40 [4];
  undefined4 *local_30;
  undefined4 *local_2c;
  
  uVar14 = 0;
  iVar10 = 0;
  iVar12 = 2;
  do {
    (&DAT_8006567c)[iVar10] = iVar10 < param_3;
    if (iVar10 < param_3) {
      do {
        do {
          iVar1 = FUN_80017160/*0x80017160*/();
          uVar9 = iVar1 * 0xc >> 0xf;
        } while ((param_2 & 1 << (uVar9 & 0x1f)) != 0);
        if (iVar10 < 1) break;
        puVar2 = &DAT_80065674;
        iVar1 = 0;
        do {
          iVar6 = iVar1;
          if ((int)(char)puVar2[2] == uVar9) break;
          iVar6 = iVar1 + 1;
          puVar2 = &DAT_80065675 + iVar1;
          iVar1 = iVar6;
        } while (iVar6 < iVar10);
      } while (iVar6 < iVar10);
      (&DAT_80065674)[iVar12] = (char)uVar9;
    }
    (&DAT_80065674)[iVar12] = 0xff;
    iVar10 = iVar10 + 1;
    iVar12 = iVar12 + 1;
  } while (iVar10 < 6);
  local_res4 = param_2;
  if ((_DAT_80065908 & 0x20) != 0) {
    local_res4 = 0;
  }
  iVar1 = 0;
  FUN_8001d370/*0x8001d370*/(1,&DAT_80100774,0,0);
  FUN_80016da8/*0x80016da8*/(auStack_60);
  FUN_8004d314/*0x8004d314*/(auStack_60,&DAT_80100798);
  uVar3 = FUN_8001a2cc/*0x8001a2cc*/(param_4,0x70,0xfffffff3,0xffffffff);
  iVar12 = FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 8),1);
  func_0x8001a4ac(uVar3);
  *(undefined1 *)(iVar12 + 4) = 0x80;
  *(undefined1 *)(iVar12 + 5) = 0x80;
  *(undefined1 *)(iVar12 + 6) = 0x80;
  iVar10 = param_4;
  if (0 < param_3) {
    do {
      DAT_80111e06 = *(short *)(iVar10 + 2) + *(short *)(iVar10 + 6);
      uVar3 = FUN_8001a2cc/*0x8001a2cc*/(iVar10,0x70,0xfffffff3,0xffffffff);
      uVar4 = func_0x8001a24c(&DAT_80111e04);
      local_40[iVar1] = uVar4;
      FUN_80019e20/*0x80019e20*/();
      FUN_80019a58/*0x80019a58*/(iVar12,(&PTR_DAT_80111e0c)[(char)(&DAT_8006567c)[iVar1]],&DAT_80111e04,10);
      puVar5 = (uint *)func_0x80021c20((short)(char)(&DAT_80065676)[iVar1]);
      *puVar5 = *puVar5 | 8;
      uVar4 = FUN_8001bda0/*0x8001bda0*/(_DAT_800737d4,0xb);
      FUN_8003e598/*0x8003e598*/(puVar5,uVar4);
      FUN_8001dc1c/*0x8001dc1c*/(puVar5);
      puVar5[0x14] = 0x4000000;
      puVar5[0x12] = 0x4000000;
      *(undefined2 *)((int)puVar5 + 0x42) = 0x4e3;
      puVar5[0x13] = puVar5[0x13] + 0x2ff800;
      FUN_8001d708/*0x8001d708*/(puVar5);
      FUN_8001d9c0/*0x8001d9c0*/(auStack_60,0x100);
      FUN_8001a4f8/*0x8001a4f8*/(uVar3,0);
      Buffer_StartOTagOther/*0x80011a10*/();
      FUN_8001de08/*0x8001de08*/(puVar5);
      DrawOTag/*0x8004fb74*/(_DAT_80065910 + 0x3ffc);
      DrawSync/*0x8004f580*/(0);
      func_0x8001a584(uVar3);
      func_0x8001a4ac(uVar3);
      iVar1 = iVar1 + 1;
      FUN_8001a994/*0x8001a994*/(puVar5[0x16]);
      func_0x8001d490(puVar5);
      iVar10 = iVar10 + 8;
    } while (iVar1 < param_3);
  }
  local_30 = local_40;
  do {
    iVar1 = (int)uVar14 / 2;
    iVar10 = iVar1 * 8 + param_4;
    DAT_80111e06 = *(short *)(iVar10 + 2) + *(short *)(iVar10 + 6);
    Buffer_StartOTagOther/*0x80011a10*/();
    FUN_80019e20/*0x80019e20*/();
    uVar3 = FUN_8001a2cc/*0x8001a2cc*/(iVar10,0x70,0xfffffff3,0xffffffff,0);
    puVar5 = (uint *)func_0x80021c20((short)(char)(&DAT_80065676)[iVar1]);
    *puVar5 = *puVar5 | 8;
    uVar4 = FUN_8001bda0/*0x8001bda0*/(_DAT_800737d4,0xb);
    FUN_8003e598/*0x8003e598*/(puVar5,uVar4);
    FUN_8001dc1c/*0x8001dc1c*/(puVar5);
    puVar5[0x14] = 0x4000000;
    puVar5[0x12] = 0x4000000;
    *(undefined2 *)((int)puVar5 + 0x42) = 0x4e3;
    puVar5[0x13] = puVar5[0x13] + 0x2ff800;
    FUN_8001d708/*0x8001d708*/(puVar5);
    FUN_8001d9c0/*0x8001d9c0*/(auStack_60,0x100);
    FUN_8001d370/*0x8001d370*/();
    local_2c = local_30 + iVar1;
    pcVar13 = &DAT_8006567c + iVar1;
    do {
      FUN_80019e20/*0x80019e20*/();
      func_0x8001a2ac(*local_2c,(int)DAT_80111e04,(int)DAT_80111e06);
      *(undefined1 *)(iVar12 + 4) = 0x80;
      *(undefined1 *)(iVar12 + 5) = 0x80;
      *(undefined1 *)(iVar12 + 6) = 0x80;
      FUN_80019a58/*0x80019a58*/(iVar12,(&PTR_DAT_80111e0c)[*pcVar13],&DAT_80111e04,10);
      *(undefined1 *)(iVar12 + 4) = 0x7c;
      *(undefined1 *)(iVar12 + 5) = 0x5f;
      *(undefined1 *)(iVar12 + 6) = 0x16;
      if ((uVar14 & 1) != 0) {
        FUN_80019a58/*0x80019a58*/(iVar12,&DAT_801007a8,&DAT_80111e04,8);
        FUN_80019a58/*0x80019a58*/(iVar12,&DAT_801007ac,&DAT_80111e04,9);
      }
      do {
        Buffer_StartOTagOther/*0x80011a10*/();
        FUN_8001a4f8/*0x8001a4f8*/(uVar3,0);
        if ((uVar14 & 1) == 0) {
          FUN_80019a58/*0x80019a58*/(iVar12,&DAT_801007a8,&DAT_80111e1c,8);
          FUN_80019a58/*0x80019a58*/(iVar12,&DAT_801007ac,&DAT_80111e1c,9);
        }
        FUN_8001d9c0/*0x8001d9c0*/(auStack_60,0x100);
        if (*pcVar13 != '\0') {
          FUN_8001de08/*0x8001de08*/(puVar5);
        }
        DrawOTag/*0x8004fb74*/(_DAT_80065910 + 0x3ffc);
        RotMatrixY/*0x8004d914*/(0x10,puVar5 + 4);
        if ((code *)puVar5[0x19] != (code *)0x0) {
          (*(code *)puVar5[0x19])(puVar5,0,0);
        }
        puVar5[0xb] = 0x4000000;
        puVar5[9] = 0x4000000;
        VSync/*0x80047e44*/(0);
        DrawSync/*0x8004f580*/(0);
        func_0x8001a584(uVar3);
        SetDispMask/*0x8004f4e8*/(1);
        Pad_Tick/*0x800120d4*/();
        uVar9 = _DAT_80065930 | _DAT_80065934;
      } while ((uVar9 & 0xf8d00000) == 0);
      FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,(uint)((uVar9 & 0x8400000) != 0) << 3);
      iVar10 = (uVar9 >> 0x1d & 1) + ((int)uVar9 >> 0x1f);
      if ((uVar14 & 1) != 0) {
        iVar8 = *pcVar13 + iVar10 + 4;
        iVar6 = iVar8;
        if (iVar8 < 0) {
          iVar6 = *pcVar13 + iVar10 + 7;
        }
        *pcVar13 = (char)iVar8 + (char)(iVar6 >> 2) * -4;
      }
      if (iVar10 != 0) {
        do {
          uVar7 = ((char)(&DAT_80065676)[iVar1] + iVar10 + 0xc) % 0xc;
          (&DAT_80065676)[iVar1] = (char)uVar7;
        } while ((local_res4 & 1 << (uVar7 & 0x1f)) != 0);
      }
      iVar10 = param_3 << 1;
      if ((uVar9 & 0x10000000) != 0) {
        iVar10 = 1;
      }
      uVar14 = (int)((uVar9 >> 0x1e & 1) + uVar14 + iVar10) % iVar10;
    } while (((int)uVar14 / 2 == iVar1) && ((uVar9 & 0x8d00000) == 0));
    FUN_8001a4f8/*0x8001a4f8*/(uVar3,0);
    FUN_8001d9c0/*0x8001d9c0*/(auStack_60,0x100);
    Buffer_StartOTagOther/*0x80011a10*/();
    if ((&DAT_8006567c)[iVar1] != '\0') {
      FUN_8001de08/*0x8001de08*/(puVar5);
    }
    DrawOTag/*0x8004fb74*/(_DAT_80065910 + 0x3ffc);
    DrawSync/*0x8004f580*/(0);
    func_0x8001a584(uVar3);
    FUN_8001a994/*0x8001a994*/(puVar5[0x16]);
    func_0x8001d490(puVar5);
    func_0x8001a4ac(uVar3);
    FUN_80019e20/*0x80019e20*/();
    *(undefined1 *)(iVar12 + 4) = 0x80;
    *(undefined1 *)(iVar12 + 5) = 0x80;
    *(undefined1 *)(iVar12 + 6) = 0x80;
    func_0x8001a2ac(local_30[iVar1],(int)DAT_80111e04,(int)DAT_80111e06);
    FUN_80019a58/*0x80019a58*/(iVar12,(&PTR_DAT_80111e0c)[(char)(&DAT_8006567c)[iVar1]],&DAT_80111e04,10);
    if ((uVar9 & 0x8400000) != 0) {
      iVar10 = 0;
      if ((((0 < param_3) && (DAT_8006567c == '\0')) && (iVar10 = 1, 1 < param_3)) &&
         (DAT_8006567d == '\0')) {
        iVar1 = 2;
        do {
          iVar10 = iVar1;
          if (param_3 <= iVar10) break;
          iVar1 = iVar10 + 1;
        } while ((&DAT_8006567c)[iVar10] == '\0');
      }
      if (iVar10 == param_3) {
        if ((DAT_80065319 == '\x01') && ((_DAT_80065908 & 4) != 0)) {
          DAT_0000531a = 2;
        }
        uVar9 = 0;
        FUN_8004445c/*0x8004445c*/(1,DAT_8011338c,0xb);
      }
    }
    if ((uVar9 & 0x8d00000) != 0) {
      iVar10 = 0;
      if (0 < param_3) {
        puVar11 = local_40;
        do {
          uVar3 = *puVar11;
          puVar11 = puVar11 + 1;
          FUN_800183ec/*0x800183ec*/(uVar3);
          iVar10 = iVar10 + 1;
        } while (iVar10 < param_3);
      }
      FUN_800190a8/*0x800190a8*/(iVar12);
      return;
    }
  } while( true );
}

