// addr: 0x80100c30  name: FUN_80100c30

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100c30(uint *param_1,undefined4 param_2,uint *param_3)

{
  char cVar1;
  undefined2 uVar2;
  undefined4 *puVar3;
  uint *puVar4;
  int iVar5;
  uint uVar6;
  undefined4 *puVar7;
  undefined4 *puVar8;
  undefined4 uVar9;
  uint uVar10;
  undefined4 uVar11;
  uint uVar12;
  undefined4 uVar13;
  
  switch(param_2) {
  case 0:
    if ((param_3 != (uint *)0x0) &&
       (uVar6 = param_1[0x20], param_1[0x20] = uVar6 - (int)param_3, (int)(uVar6 - (int)param_3) < 0
       )) {
      do {
        uVar6 = param_1[0x21];
        puVar3 = (undefined4 *)((uVar6 & 7) * 0x80 + param_1[0x22]);
        puVar7 = puVar3 + 0x20;
        param_1[0x20] = param_1[0x20] + 6;
        puVar8 = (undefined4 *)&DAT_8008f660;
        do {
          uVar9 = puVar3[1];
          uVar11 = puVar3[2];
          uVar13 = puVar3[3];
          *puVar8 = *puVar3;
          puVar8[1] = uVar9;
          puVar8[2] = uVar11;
          puVar8[3] = uVar13;
          puVar3 = puVar3 + 4;
          puVar8 = puVar8 + 4;
        } while (puVar3 != puVar7);
        param_1[0x21] = uVar6 + 1;
      } while ((int)param_1[0x20] < 0);
    }
    DAT_80101b88 = FUN_80017160/*0x80017160*/();
  case 1:
    *param_1 = 0x80;
    param_1[0x22] = 0x80;
    uVar9 = FUN_8001ffd4/*0x8001ffd4*/(0x80065a50,0x100);
    iVar5 = func_0x8003d080(0x7f000000,uVar9);
    _DAT_80065a10 = (uint)(iVar5 != 0);
  case 2:
    func_0x80023d00();
    FUN_80020890/*0x80020890*/(param_1,0xf0);
    param_1 = (uint *)0x1;
  case 0xb:
    param_1 = (uint *)FUN_8004410c/*0x8004410c*/(param_1);
    FUN_800443c8/*0x800443c8*/(param_1,*(undefined4 *)(*(int *)(_DAT_800659fc + 0x58) + 8),1,0);
  case 0x11:
    func_0x80022120(param_1,param_3);
  case 9:
    cVar1 = '\b';
    if (0x540000 < param_3[2] + 0xfb140000) {
      puVar4 = (uint *)FUN_8001d5e0/*0x8001d5e0*/(param_1);
      if ((*puVar4 & 0x2000000) != 0) {
        return 0;
      }
      FUN_8002c958/*0x8002c958*/(puVar4,0xffffff6a,&DAT_80100098,0);
      uVar9 = *(undefined4 *)(_DAT_800659fc + 0x58);
      for (uVar6 = puVar4[0xe]; uVar6 != 0; uVar6 = *(uint *)(uVar6 + 0x34)) {
        *(undefined2 *)(*(int *)(uVar6 + 0x30) + 0x28) = 0x40;
      }
      if (puVar4[0x1a] != 0) {
        *(undefined2 *)(puVar4[0x1a] + 0x28) = 0x40;
      }
      cVar1 = *(char *)((int)puVar4 + 0xd3);
      if (cVar1 == '\0') {
        cVar1 = FUN_8004410c/*0x8004410c*/(0,uVar9,0x24d,0x24c,4,0x3c);
        *(char *)((int)puVar4 + 0xd3) = cVar1;
      }
      FUN_800443c8/*0x800443c8*/(cVar1,*(undefined4 *)(_DAT_800737e8 + 8),3,0);
      FUN_80044574/*0x80044574*/((int)*(char *)((int)puVar4 + 5),0);
      puVar4[0x19] = (uint)FUN_801006cc;
      *(undefined1 *)(puVar4 + 2) = 0;
      *puVar4 = *puVar4 & 0xfffffffd | 0x3000020;
      iVar5 = FUN_80017160/*0x80017160*/();
      *(char *)((int)puVar4 + 0xd2) = (char)((iVar5 << 2) >> 0xf) + '<';
      puVar4[0x20] = 0;
      puVar4[0x21] = 0x1c980;
      puVar4[0x22] = 0;
      FUN_80020890/*0x80020890*/(puVar4);
      uVar6 = puVar4[0x38];
      if (uVar6 == 0) {
        return 0;
      }
      param_1 = (uint *)Heap_AllocOrRetry/*0x800116f4*/(0x40);
      iVar5 = FUN_80021888/*0x80021888*/(*(undefined1 *)((int)puVar4 + 0xd2));
      uVar10 = *(uint *)(uVar6 + 0x4c);
      uVar12 = *(uint *)(uVar6 + 0x50);
      *param_1 = *(uint *)(uVar6 + 0x48);
      param_1[1] = uVar10;
      param_1[2] = uVar12;
      param_1[3] = 0x78;
      param_1[4] = 0x39f0000;
      param_1[6] = 0x5470000;
      param_1[5] = 0x20f800;
      param_1[7] = 0x168;
      param_1[8] = *(uint *)(iVar5 + 0x48);
      param_1[9] = *(int *)(iVar5 + 0x4c) - 0x12000;
      param_1[10] = *(int *)(iVar5 + 0x50) + 0x50000;
      param_1[0xb] = 0;
      func_0x8003dbb0(uVar6);
      cVar1 = '\0';
    }
    if (((char)param_1[1] != cVar1) || (DAT_80101b88 = DAT_80101b88 + 1, (DAT_80101b88 & 0xf) != 0))
    {
      return 0;
    }
    iVar5 = FUN_8001d5a0/*0x8001d5a0*/(param_1);
    if (*(int *)(iVar5 + 0x8c) < 0xee6) {
      param_1 = (uint *)FUN_8001ac44/*0x8001ac44*/(*(undefined4 *)(_DAT_800659fc + 0x58),0x24b,0x80,8);
    }
    uVar9 = 0x24a;
    if ((*param_1 & 0x80000) != 0) {
      uVar9 = 0x249;
    }
    param_1 = (uint *)FUN_8001ac44/*0x8001ac44*/(*(undefined4 *)(_DAT_800659fc + 0x58),uVar9,0x80,8);
    uVar2 = Math_Atan2_Pos/*0x80016c88*/(iVar5 + 0x10);
    *(undefined2 *)((int)param_1 + 0x42) = uVar2;
    *(undefined1 *)(param_1 + 1) = 1;
    *param_1 = 0x24;
    uVar6 = param_3[1];
    uVar10 = param_3[2];
    param_1[0x12] = *param_3;
    param_1[0x13] = uVar6;
    param_1[0x14] = uVar10;
    param_1[0x19] = 0x8003e80c;
    FUN_8002036c/*0x8002036c*/();
  case 7:
    FUN_8001d470/*0x8001d470*/(0x8c);
  case 4:
    Heap_Free/*0x80045088*/(param_1[0x22]);
    return 0;
  default:
    return 0;
  }
}

