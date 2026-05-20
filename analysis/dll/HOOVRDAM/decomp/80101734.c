// addr: 0x80101734  name: FUN_80101734

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80101734(uint *param_1,uint param_2,uint *param_3)

{
  char cVar1;
  undefined4 uVar2;
  int iVar3;
  uint uVar4;
  uint *puVar5;
  int iVar6;
  uint *puVar7;
  uint uVar8;
  
  if (param_2 == 3) {
LAB_80101780:
    puVar7 = (uint *)*param_3;
    if (((char)puVar7[1] == '\x02') && ((*param_1 & 0x10000) != 0)) {
      if ((*puVar7 & 0x2000000) != 0) {
        return 0;
      }
      cVar1 = *(char *)((int)puVar7 + 0xd3);
      if (cVar1 == '\0') {
        cVar1 = FUN_8004410c/*0x8004410c*/();
        *(char *)((int)puVar7 + 0xd3) = cVar1;
      }
      FUN_800443c8/*0x800443c8*/(cVar1,*(undefined4 *)(param_1[0x16] + 8),3,0);
      uVar2 = FUN_8004410c/*0x8004410c*/();
      FUN_800447e8/*0x800447e8*/(uVar2,*(undefined4 *)(param_1[0x16] + 8),4,puVar7 + 9);
      FUN_80044574/*0x80044574*/((int)*(char *)((int)puVar7 + 5),0);
      puVar7[0x19] = (uint)FUN_801006cc;
      *(undefined1 *)(puVar7 + 2) = 8;
      *puVar7 = *puVar7 & 0xfffffffd | 0x3000020;
      puVar5 = (uint *)0x3000020;
      do {
        iVar3 = FUN_80017160/*0x80017160*/(puVar5);
        uVar4 = ((iVar3 << 2) >> 0xf) + 0x3c;
        *(char *)((int)puVar7 + 0xd2) = (char)uVar4;
        puVar5 = puVar7;
      } while ((uVar4 & 0xff) == (int)*(short *)((int)param_1 + 6));
      FUN_80020890/*0x80020890*/(puVar7,0x3c);
      param_1 = (uint *)puVar7[0x38];
      if (param_1 == (uint *)0x0) {
        return 0;
      }
      puVar5 = (uint *)Heap_AllocOrRetry/*0x800116f4*/(0x40);
      iVar3 = FUN_80021888/*0x80021888*/(*(undefined1 *)((int)puVar7 + 0xd2));
      uVar4 = param_1[0x13];
      uVar8 = param_1[0x14];
      *puVar5 = param_1[0x12];
      puVar5[1] = uVar4;
      puVar5[2] = uVar8;
      puVar5[3] = 0x78;
      puVar5[4] = 0x39f0000;
      puVar5[6] = 0x5210000;
      puVar5[5] = 0x25f800;
      puVar5[7] = 0xf0;
      puVar5[8] = *(uint *)(iVar3 + 0x48);
      param_3 = puVar5 + 8;
      puVar5[9] = *(int *)(iVar3 + 0x4c) - 0x12000;
      iVar6 = -0x50000;
      if (*(int *)(iVar3 + 0x50) < 0x5210000) {
        iVar6 = 0x50000;
      }
      puVar5[10] = *(int *)(iVar3 + 0x50) + iVar6;
      puVar5[0xb] = 0;
      func_0x8003dbb0(param_1);
    }
    FUN_8002239c/*0x8002239c*/(param_1,param_3);
  }
  else {
    if ((param_2 < 4) && (param_2 == 2)) goto LAB_80101a6c;
    if (param_2 != 8) goto LAB_80101780;
  }
  if (*(char *)(_DAT_1f80000c + 4) == '\x02') {
    iVar3 = *(int *)(_DAT_1f80000c + 0x80);
    uVar4 = 0x50000;
    if (*(short *)((int)param_1 + 0x42) != 0) {
      iVar3 = -iVar3;
      uVar4 = (uint)(0x5f580 < iVar3);
      if (uVar4 == 0) goto LAB_80101a44;
    }
    if ((int)(uVar4 | 0xf580) < iVar3) {
      iVar3 = (*(int *)(_DAT_1f80000c + 0x28) + 0x12000) - param_1[0x13];
      if (iVar3 < 0) {
        iVar3 = -iVar3;
      }
      if (iVar3 < 0x10000) {
        iVar3 = *(int *)(_DAT_1f80000c + 0x2c) - param_1[0x14];
        if (iVar3 < 0) {
          iVar3 = -iVar3;
        }
        if (iVar3 < 0xa000) {
          *param_1 = *param_1 | 0x10000;
          FUN_80020890/*0x80020890*/(param_1,0);
        }
      }
    }
  }
LAB_80101a44:
  *param_1 = *param_1 & 0xfffeffff;
  FUN_80022320/*0x80022320*/(param_1,param_3);
LAB_80101a6c:
  *param_1 = *param_1 & 0xfffeffff;
  return 0;
}

