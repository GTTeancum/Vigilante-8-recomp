// addr: 0x8010167c  name: FUN_8010167c

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_8010167c(int param_1,undefined4 param_2,uint param_3)

{
  bool bVar1;
  undefined1 uVar2;
  char cVar3;
  int iVar4;
  undefined4 *puVar5;
  undefined1 *puVar6;
  int iVar7;
  uint uVar8;
  uint uVar9;
  int iVar10;
  int *piVar11;
  undefined4 uVar12;
  uint uVar13;
  int iVar14;
  int iVar15;
  uint *puVar16;
  uint local_240;
  int local_23c;
  short local_238;
  short local_236;
  short local_234;
  undefined1 auStack_230 [32];
  undefined1 auStack_210 [64];
  undefined1 auStack_1d0 [12];
  undefined2 *local_1c4;
  undefined4 local_1c0;
  undefined1 auStack_1b8 [3];
  undefined1 local_1b5;
  undefined1 local_1b4;
  undefined1 local_1b3;
  undefined1 local_1b2;
  undefined1 local_1b1;
  undefined2 local_1b0;
  undefined2 local_1ae;
  undefined2 local_1ac;
  undefined2 local_1aa;
  undefined1 auStack_1a8 [8];
  short local_1a0;
  short local_19e;
  undefined1 auStack_190 [96];
  undefined1 auStack_130 [256];
  int local_30 [2];
  
  iVar4 = Asset_LoadFileOrDie/*0x80015f80*/(s_Shell_Load_tbl_80100048);
  _DAT_80065328 = 0;
  DAT_80107da0 = &DAT_80107da4;
  DAT_80107da4 = 0;
  DAT_80107da8 = &DAT_80107da0;
  DAT_80107d90 = &DAT_80107d94;
  DAT_80107d94 = 0;
  DAT_80107d98 = &DAT_80107d90;
  SetDefDispEnv/*0x8004f198*/(auStack_1a8,0,0,0x140,0xf0);
  SetDefDrawEnv/*0x8004f0e4*/(auStack_190,0,0,0x140,0xf0);
  local_1a0 = (short)DAT_8006531c;
  local_19e = (short)DAT_8006531d;
  PutDrawEnv/*0x8004fbe4*/(auStack_190);
  PutDispEnv/*0x8004fdb0*/(auStack_1a8);
  local_1b5 = 3;
  local_1b1 = 0x60;
  local_1b4 = 0xff;
  local_1ae = 0xc2;
  local_1b3 = 0;
  local_1b2 = 0;
  local_1b0 = 0;
  local_1aa = 0xc;
  FUN_800185cc/*0x800185cc*/(auStack_1d0,iVar4 + *(int *)(iVar4 + 0xc));
  *local_1c4 = 0xdd;
  local_1c4[1] = 0xb6;
  LoadImage/*0x8004f82c*/(local_1c4,local_1c0);
  FUN_80019f44/*0x80019f44*/(0,0xb9,0xdb,0xb9,0x525252);
  FUN_80019f44/*0x80019f44*/(0,0xba,0xdb,0xba,0x2988);
  FUN_80019f44/*0x80019f44*/(0,0xd5,0xe2,0xd5,0x2988);
  FUN_80019f44/*0x80019f44*/(0,0xd6,0xe3,0xd6,0x525252);
  _DAT_80065310 = 0;
  puVar5 = (undefined4 *)strcpy/*0x800524f4*/(auStack_210,param_1);
  uVar2 = DAT_8010005c;
  *puVar5 = DAT_80100058;
  *(undefined1 *)(puVar5 + 1) = uVar2;
  _DAT_80065a38 = Overlay_LoadAndRelocate/*0x80011adc*/(auStack_210,0x2e);
  iVar14 = param_1;
  if (_DAT_80065a38 != 0) {
    iVar14 = _DAT_80065a38 + 1;
  }
  puVar6 = (undefined1 *)strcpy/*0x800524f4*/(auStack_210,iVar14);
  if (puVar6 == (undefined1 *)0x0) {
    iVar14 = FUN_80052544/*0x80052544*/(auStack_210,0x2e);
    puVar6 = auStack_210 + iVar14;
  }
  *puVar6 = 0;
  iVar14 = func_0x80011aa8(0,auStack_210);
  if ((iVar14 == 0) &&
     (iVar14 = func_0x80011a38(*(undefined4 *)(_DAT_80065a38 + 4),auStack_210), iVar14 == 0)) {
    iVar14 = -0x7ffddd58;
  }
  _DAT_80065a34 = iVar14;
  _DAT_800659fc = FUN_80021b80/*0x80021b80*/(iVar14,0,0,0);
  *(int *)(_DAT_800659fc + 100) = _DAT_80065a34;
  Stream_OpenByName/*0x800159b4*/(param_1);
  Iff_ReadChunkData/*0x800225d4*/(&local_240,local_30);
  iVar14 = local_23c;
  local_30[0] = local_23c;
  if (local_23c != 0) {
    do {
      iVar7 = Iff_ReadChunkData/*0x800225d4*/(&local_240,local_30);
      local_1ac = (undefined2)(((iVar14 - local_30[0]) * 0xda) / iVar14);
      DrawPrim/*0x8004fb18*/(auStack_1b8);
      DrawSync/*0x8004f580*/(0);
      uVar13 = local_240 >> 0x18 | local_240 >> 8 & 0xff00 | (local_240 & 0xff00) << 8 |
               local_240 << 0x18;
      iVar10 = local_23c;
      if (uVar13 != 0x53554e41) {
        uVar8 = 0x58450000;
        if (uVar13 < 0x53554e42) {
          if ((uVar13 != 0x48454144) &&
             ((uVar8 = 0x504c0000, 0x48454144 < uVar13 ||
              ((uVar13 != 0x42535020 &&
               (((uVar8 = 0x434f0000, 0x42535020 < uVar13 ||
                 (uVar8 = 0x41494d50, uVar13 != 0x41494d50)) &&
                (uVar8 = uVar8 | 0x4c53, uVar13 != uVar8)))))))) {
            if (uVar13 == (uVar8 | 0x5458)) goto LAB_80101d2c;
            uVar9 = 0x52450000;
            if ((((uVar8 | 0x5458) < uVar13) || (uVar9 = 0x4a554e43, uVar13 != 0x4a554e43)) &&
               ((uVar13 != (uVar9 | 0x4354) && (uVar8 = 0x52534547, uVar13 != 0x52534547))))
            goto LAB_80101b34;
          }
        }
        else {
LAB_80101b34:
          if (((uVar13 != (uVar8 | 0x4e56)) &&
              (((uVar9 = 0x58520000, (uVar8 | 0x4e56) < uVar13 ||
                ((((uVar13 != 0x54494e46 &&
                   ((uVar8 = 0x58420000, 0x54494e46 < uVar13 ||
                    (uVar8 = 0x54455854, uVar13 != 0x54455854)))) && (uVar13 != (uVar8 | 0x474d)))
                 && (uVar9 = 0x58424d50, uVar13 != 0x58424d50)))) && (uVar13 != (uVar9 | 0x5450)))))
             && ((((uVar8 = 0x5a4d0000, uVar13 <= (uVar9 | 0x5450) &&
                   (uVar8 = 0x584c5343, uVar13 == 0x584c5343)) ||
                  ((uVar13 != (uVar8 | 0x4150) && (uVar13 != 0x5a4f4e45)))) &&
                 (iVar10 = 0x43, *(byte *)(iVar7 + 3) == param_3)))) {
            iVar10 = FUN_80019034/*0x80019034*/(iVar4 + *(int *)(iVar4 + 8),0x43);
            *(undefined1 *)(iVar10 + 4) = 0x40;
            *(undefined1 *)(iVar10 + 5) = 0x40;
            *(undefined1 *)(iVar10 + 6) = 0x40;
            FUN_80019960/*0x80019960*/(iVar10,(&PTR_s_Secret_Base_80106d94)[DAT_800658f8],0x13,0xe);
            FUN_80019010/*0x80019010*/(iVar10,0x22);
            *(undefined1 *)(iVar10 + 4) = 0x80;
            *(undefined1 *)(iVar10 + 5) = 0x80;
            *(undefined1 *)(iVar10 + 6) = 0x80;
            FUN_80019960/*0x80019960*/(iVar10,(&PTR_s_Secret_Base_80106d94)[DAT_800658f8],0x10,0xb);
            FUN_800190a8/*0x800190a8*/(iVar10);
            piVar11 = (int *)FUN_80019034/*0x80019034*/(iVar4 + *(int *)(iVar4 + 4),1);
            *(undefined1 *)(piVar11 + 1) = 0x60;
            *(undefined1 *)((int)piVar11 + 5) = 0x60;
            *(undefined1 *)((int)piVar11 + 6) = 0x28;
            iVar10 = func_0x80019234(piVar11,param_2,auStack_130,0x120);
            FUN_80019960/*0x80019960*/(piVar11,auStack_130,0x10,
                            (int)(0x40 - iVar10 * (uint)*(byte *)(*piVar11 + 7)) / 2 + 0x73);
            FUN_800190a8/*0x800190a8*/(piVar11);
            Heap_Free/*0x80045088*/(iVar4);
LAB_80101d2c:
            iVar15 = 0;
            piVar11 = (int *)&DAT_800737a0;
            do {
              if (*piVar11 != 0) {
                FUN_8001a91c/*0x8001a91c*/();
              }
              iVar15 = iVar15 + 1;
              piVar11 = piVar11 + 1;
              iVar10 = local_23c;
            } while (iVar15 < 0x10);
          }
        }
      }
      Heap_Free/*0x80045088*/(iVar7,iVar10);
    } while (local_30[0] != 0);
  }
  Stream_Close/*0x80015a00*/();
  FUN_80017e0c/*0x80017e0c*/();
  local_238 = (short)(_DAT_80065ab0 * 0x1800 >> 0xc);
  local_236 = (short)(_DAT_80065ab2 * 0x1800 >> 0xc);
  local_234 = (short)(_DAT_80065ab4 * 0x1800 >> 0xc);
  _DAT_800659d0 = 0;
  func_0x8001d404(0,&local_238,_DAT_80065b08);
  func_0x8001d404(1,&DAT_801000e8,_DAT_80065af8);
  local_238 = -_DAT_80065ab0;
  local_234 = -_DAT_80065ab4;
  local_236 = _DAT_80065ab2;
  func_0x8001d404(2,&local_238,_DAT_80065b10);
  FUN_80016da8/*0x80016da8*/(auStack_230);
  SetColorMatrix/*0x8004d3a4*/(0x8006f760);
  SetLightMatrix/*0x8004d374*/(0x8006f720);
  SetBackColor/*0x8004d4e4*/(0x40,0x40,0x40);
  iVar14 = 0;
  iVar4 = *DAT_80107da0;
  piVar11 = DAT_80107da0;
  while (iVar4 != 0) {
    piVar11 = (int *)*piVar11;
    iVar14 = iVar14 + 1;
    iVar4 = *piVar11;
  }
  local_1b4 = 0;
  local_1b3 = 0xff;
  local_1b2 = 0;
  local_30[0] = iVar14;
  if ((int **)DAT_80107da8 != &DAT_80107da0) {
    do {
      DAT_80107da0 = (int *)*DAT_80107da0;
      DAT_80107da0[1] = (int)&DAT_80107da0;
      local_30[0] = local_30[0] + -1;
      local_1ac = (undefined2)(((iVar14 - local_30[0]) * 0xda) / iVar14);
      DrawPrim/*0x8004fb18*/(auStack_1b8);
    } while ((int **)DAT_80107da8 != &DAT_80107da0);
  }
  FUN_80020658/*0x80020658*/(&DAT_80107d90);
  if (DAT_80065319 == '\0') {
    puVar16 = *(uint **)(DAT_80065904 * 0x10 + *(int *)(_DAT_8006590c + DAT_80065674 * 8 + 8) + 8);
    iVar14 = 0;
    iVar4 = 0;
    _DAT_80065acc = _DAT_80065acc & 0xffffff00;
    do {
      if ((int)(uint)*(ushort *)
                      (DAT_80065904 * 0x10 + *(int *)(_DAT_8006590c + DAT_80065674 * 8 + 8) + 6) <=
          iVar4) break;
      cVar3 = (char)*puVar16;
      uVar13 = (int)cVar3 & 0x80;
      if ((int)cVar3 == 0xffffffff) {
        if (_DAT_80065ad4 == 0) {
          uVar13 = func_0x80021f30(puVar16);
          _DAT_00000007 = (short)cVar3;
          _DAT_80065ad4 = uVar13;
          goto LAB_80102174;
        }
      }
      else {
LAB_80102174:
        if (uVar13 != 0) {
          _DAT_80065acc = *puVar16;
          _DAT_80065ad0 = (undefined2)puVar16[1];
          iVar4 = iVar4 + 1;
        }
        iVar10 = func_0x80021f30(puVar16);
        if (iVar10 != 0) {
          *(undefined1 *)(iVar10 + 8) = 4;
          iVar14 = iVar14 + 1;
          bVar1 = DAT_8006531a == '\0';
          *(short *)(iVar10 + 6) = (short)iVar14;
          if (bVar1) {
            func_0x8002e604(iVar10,*(ushort *)(iVar10 + 0xc) >> 1);
          }
          FUN_8002036c/*0x8002036c*/(iVar10);
        }
      }
      iVar4 = iVar4 + 1;
      puVar16 = (uint *)((int)puVar16 + 6);
    } while (iVar14 < 6);
  }
  if (_DAT_80065ad4 == 0) {
    _DAT_80065ad4 = func_0x80021fd8(0xffffffff);
    FUN_8002036c/*0x8002036c*/(_DAT_80065ad4);
  }
  if (('\x02' < DAT_80065319) && (_DAT_80065ad8 == 0)) {
    _DAT_80065ad8 = func_0x80021fd8(0xfffffffe);
    FUN_8002036c/*0x8002036c*/(_DAT_80065ad8);
  }
  cVar3 = FUN_8004410c/*0x8004410c*/();
  *(char *)(_DAT_80065ad4 + 5) = cVar3;
  FUN_800443c8/*0x800443c8*/((int)cVar3,*(undefined4 *)(_DAT_800737e0 + 8),0,0);
  puVar5 = (undefined4 *)&DAT_80065ad4;
  iVar14 = FUN_80017160/*0x80017160*/();
  CD_PlayTrack/*0x80043ce0*/((int)(iVar14 * (uint)DAT_80065bfc) >> 0xf);
  FUN_8002ea94/*0x8002ea94*/(_DAT_80065ad4,1);
  uVar12 = func_0x8003d918(_DAT_80065ad4,0x100);
  *(undefined4 *)(_DAT_80065ad4 + 0xe0) = uVar12;
  FUN_80020744/*0x80020744*/();
  func_0x8003da98(*(undefined4 *)(_DAT_80065ad4 + 0xe0));
  if (DAT_8006531a < '\x02') {
    uVar13 = (uint)*(ushort *)(_DAT_80065ad4 + 0xc);
    if (DAT_8006531a == '\0') {
      puVar5 = (undefined4 *)0x1;
      uVar13 = uVar13 << 1;
    }
    func_0x8002e604(*puVar5,(int)(uVar13 * 3) >> 1);
  }
  if (_DAT_80065ad8 != 0) {
    FUN_8002ea94/*0x8002ea94*/(_DAT_80065ad8,1);
    uVar12 = func_0x8003d918(_DAT_80065ad8,0x100);
    *(undefined4 *)(_DAT_80065ad8 + 0xe0) = uVar12;
    FUN_80020744/*0x80020744*/();
    func_0x8003da98(*(undefined4 *)(_DAT_80065ad8 + 0xe0));
    iVar14 = _DAT_80065ad8;
    uVar13 = _DAT_80065ad4;
    *(int *)(_DAT_80065ad4 + 0xe4) = _DAT_80065ad8;
    *(uint *)(iVar14 + 0xe4) = uVar13;
    cVar3 = FUN_8004410c/*0x8004410c*/();
    *(char *)(_DAT_80065ad8 + 5) = cVar3;
    FUN_800443c8/*0x800443c8*/((int)cVar3,*(undefined4 *)(_DAT_800737e4 + 8),0,0);
    if (DAT_8006531a < '\x02') {
      func_0x8002e604(_DAT_80065ad8,(uint)*(ushort *)(_DAT_80065ad8 + 0xc) << 1);
    }
  }
  DAT_80065980 = 0;
  return;
}

