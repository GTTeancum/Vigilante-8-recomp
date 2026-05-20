// addr: 0x8010d034  name: FUN_8010d034

void FUN_8010d034(undefined4 param_1,int param_2,undefined2 param_3,undefined2 param_4,
                 undefined2 param_5,undefined2 param_6,int param_7,int param_8)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  undefined1 auStack_28 [8];
  
  iVar4 = 2;
  if (param_2 != 0) {
    iVar4 = 3;
  }
  iVar2 = param_8 * iVar4 * 0x10;
  iVar3 = Iso_OpenPath/*0x800157d4*/();
  if (iVar3 == 0) {
    iVar3 = 0;
  }
  CdIntToPos/*0x80049430*/(*(undefined4 *)(iVar3 + 0xc),auStack_28);
  DAT_801133c8 = Heap_AllocOrRetry/*0x800116f4*/(param_7 * param_8);
  DAT_801133cc = Heap_AllocOrRetry/*0x800116f4*/(param_7 * param_8);
  DAT_801133dc = Heap_AllocOrRetry/*0x800116f4*/(iVar2);
  DAT_801133e0 = Heap_AllocOrRetry/*0x800116f4*/(iVar2);
  DAT_80113404 = 0;
  DAT_801133e4 = 0;
  DAT_801133f0 = param_5;
  DAT_801133f8 = 0;
  DAT_80113408 = 0;
  DAT_801133f2 = param_6;
  DAT_80113400 = (undefined2)(iVar4 << 3);
  DAT_801133e8 = param_3;
  DAT_801133ea = param_4;
  DAT_8011340c = param_2;
  DAT_801133c0 = Heap_AllocOrRetry/*0x800116f4*/(0x11000);
  DAT_801133c4 = Heap_AllocOrRetry/*0x800116f4*/(0x10000);
  StSetRing/*0x8004b8d4*/(DAT_801133c4,0x20);
  iVar4 = StSetStream/*0x8004b964*/(param_2,1,0xffffffff,0,0);
  do {
    bVar1 = iVar4 < 0;
    iVar4 = 1;
  } while (bVar1);
  return;
}

