// addr: 0x80100228  name: FUN_80100228

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80100228(uint *param_1,int param_2,int *param_3)

{
  undefined1 uVar1;
  ushort uVar2;
  uint uVar3;
  uint uVar4;
  char cVar5;
  short sVar6;
  int iVar7;
  undefined4 uVar8;
  int iVar9;
  uint uVar10;
  int iVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined8 uVar14;
  
  if (param_2 == 1) goto LAB_80100510;
  if (param_2 == 0) {
LAB_80100274:
    uVar10 = param_1[0xe];
    iVar9 = *(int *)(uVar10 + 0x38);
    iVar7 = *(int *)(iVar9 + 0x28) + *(int *)(iVar9 + 0x48);
    *(int *)(iVar9 + 0x28) = iVar7;
    if (*(char *)(iVar9 + 8) == '\0') {
      *(int *)(iVar9 + 0x48) = *(int *)(iVar9 + 0x48) + 0x10e;
      if (*(int *)(iVar9 + 0x50) < *(int *)(iVar9 + 0x28)) {
        *(int *)(iVar9 + 0x28) = *(int *)(iVar9 + 0x50);
        cVar5 = *(char *)(uVar10 + 8);
        uVar2 = *(ushort *)((int)param_1 + 0xe);
        uVar3 = param_1[2];
        uVar4 = param_1[3];
        *(undefined1 *)(iVar9 + 8) = 1;
        *(int *)(iVar9 + 0x48) =
             (int)(((uint)uVar2 * (cVar5 + -1) + (uint)(ushort)uVar4) * -0x17d7) /
             (int)((uint)uVar2 * ((char)uVar3 + -1));
        iVar7 = FUN_8001d624/*0x8001d624*/();
        uVar8 = FUN_8004410c/*0x8004410c*/();
        param_3 = (int *)0x0;
        FUN_8004483c/*0x8004483c*/(uVar8,*(undefined4 *)(param_1[0x16] + 8),0,iVar7 + 0x14);
        iVar9 = 0x17;
        iVar7 = FUN_8003fd24/*0x8003fd24*/(iVar7 + 0x14);
        goto LAB_8010035c;
      }
    }
    else {
LAB_8010035c:
      if (iVar7 < *(int *)(iVar9 + 0x4c)) {
        *(int *)(iVar9 + 0x28) = *(int *)(iVar9 + 0x4c);
        *(undefined4 *)(iVar9 + 0x48) = 0;
        *(undefined1 *)(iVar9 + 8) = 0;
      }
    }
    if ((short)param_1[3] == 0) {
      return 0;
    }
    if ((*param_1 & 0x4000) == 0) {
      return 0;
    }
    iVar7 = ratan2/*0x8004ecd4*/(*(int *)(_DAT_80065ad4 + 0x48) - param_1[0x12],
                            *(int *)(_DAT_80065ad4 + 0x50) - param_1[0x14]);
    *(ushort *)(uVar10 + 0x42) =
         *(ushort *)(uVar10 + 0x42) +
         (short)(char)(((iVar7 - (uint)*(ushort *)((int)param_1 + 0x42)) -
                       (*(ushort *)(uVar10 + 0x42) - 0x800)) * 0x100000 >> 0x18);
    FUN_8001d708/*0x8001d708*/();
LAB_801003f4:
    iVar7 = *param_3;
    uVar14 = CONCAT44((uint)*(byte *)(iVar7 + 4),7);
    if (*(byte *)(iVar7 + 4) == 2) {
      if (param_3[3] != *(int *)(param_1[0xe] + 0x38)) {
        return 0;
      }
      iVar9 = FUN_8001d624/*0x8001d624*/();
      FUN_800176f8/*0x800176f8*/(iVar7,&DAT_80100044,iVar9 + 0x14);
      uVar14 = FUN_8002c958/*0x8002c958*/(iVar7,0xfffffff4,&DAT_80100050,1);
    }
    if ((int)((ulonglong)uVar14 >> 0x20) != (int)uVar14) {
      return 0;
    }
    param_3 = (int *)(uint)*(ushort *)(iVar7 + 0xc);
  }
  else {
    if (param_2 == 3) goto LAB_801003f4;
    if (param_2 != 8) goto LAB_80100274;
  }
  sVar6 = (ushort)param_1[3] - (short)param_3;
  if ((int)(uint)(ushort)param_1[3] < (int)param_3) {
    uVar10 = param_1[0xe];
    iVar7 = *(int *)(uVar10 + 0x38);
    if (*(char *)(uVar10 + 8) == '\0') {
      return 0;
    }
    uVar1 = *(undefined1 *)(iVar7 + 8);
    uVar8 = *(undefined4 *)(iVar7 + 0x28);
    uVar12 = *(undefined4 *)(iVar7 + 0x48);
    uVar13 = *(undefined4 *)(iVar7 + 0x50);
    FUN_8003fc50/*0x8003fc50*/(uVar10);
    iVar7 = *(int *)(uVar10 + 0x38);
    if ((iVar7 != 0) &&
       (cVar5 = *(char *)(uVar10 + 8), *(char *)(uVar10 + 8) = cVar5 + -1, cVar5 != '\x01')) {
      *(undefined1 *)(iVar7 + 8) = uVar1;
      *(undefined4 *)(iVar7 + 0x28) = uVar8;
      *(undefined4 *)(iVar7 + 0x48) = uVar12;
      *(undefined4 *)(iVar7 + 0x50) = uVar13;
      *(undefined2 *)(param_1 + 3) = *(undefined2 *)((int)param_1 + 0xe);
    }
    sVar6 = FUN_80020778/*0x80020778*/(param_1);
    *(undefined2 *)(param_1 + 3) = 0;
  }
  *(short *)(param_1 + 3) = sVar6;
LAB_80100510:
  uVar10 = param_1[0xe];
  iVar11 = *(int *)(uVar10 + 0x38);
  iVar7 = FUN_8001d624/*0x8001d624*/(iVar11);
  iVar9 = Terrain_HeightAt/*0x80025400*/(*(undefined4 *)(iVar7 + 0x14),*(undefined4 *)(iVar7 + 0x1c));
  *(int *)(iVar11 + 0x50) = (iVar9 - *(int *)(iVar7 + 0x18)) + *(int *)(iVar11 + 0x4c);
  cVar5 = FUN_8003fc94/*0x8003fc94*/(uVar10);
  *(char *)(uVar10 + 8) = cVar5;
  *(char *)(param_1 + 2) = cVar5 + '\x01';
  *param_1 = *param_1 & 0xfffffffb | 0x80;
  return 0;
}

