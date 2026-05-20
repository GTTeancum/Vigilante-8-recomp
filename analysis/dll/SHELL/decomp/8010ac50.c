// addr: 0x8010ac50  name: FUN_8010ac50

void FUN_8010ac50(int param_1,int param_2)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  undefined1 *puVar4;
  int iVar5;
  short *psVar6;
  int iVar7;
  
  iVar7 = (param_1 + param_2) * 0x28;
  puVar4 = (undefined1 *)(iVar7 + DAT_801133a0);
  *(undefined2 *)((param_1 + param_2) * 0x200 + DAT_801133a4) = 0;
  *puVar4 = 0;
  iVar2 = DAT_801133a4;
  iVar5 = *(int *)(iVar7 + DAT_801133a0 + 0x18);
  iVar7 = iVar5 + 0x1fff;
  if (iVar7 < 0) {
    iVar7 = iVar5 + 0x3ffe;
  }
  iVar5 = (iVar7 >> 0xd) + -2;
  if (iVar7 >> 0xd != 1) {
    psVar6 = (short *)((param_1 + 0xf) * 0x200 + DAT_801133a4);
    do {
      iVar7 = 0xf;
      if (*psVar6 != 0x5858) {
        iVar3 = 0xe;
        do {
          iVar7 = iVar3;
          iVar3 = iVar7 + -1;
        } while (*(short *)((param_1 + iVar7) * 0x200 + DAT_801133a4) != 0x5858);
      }
      *(undefined2 *)((param_1 + iVar7) * 0x200 + iVar2) = 0;
      bVar1 = iVar5 != 0;
      iVar5 = iVar5 + -1;
    } while (bVar1);
  }
  return;
}

