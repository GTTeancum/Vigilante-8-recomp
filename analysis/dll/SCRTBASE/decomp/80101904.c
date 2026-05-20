// addr: 0x80101904  name: FUN_80101904

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80101904(int param_1,uint param_2,int *param_3)

{
  char cVar1;
  undefined4 uVar2;
  uint *puVar3;
  uint uVar4;
  char cVar5;
  int iVar6;
  
  cVar5 = '\x02';
  if (param_2 == 2) goto LAB_80101a38;
  if (param_2 < 3) {
    if (param_2 == 1) goto LAB_80101a68;
    cVar5 = '\x01';
  }
  if (param_2 == 3) {
LAB_80101960:
    puVar3 = (uint *)param_3[3];
    if (((byte)puVar3[1] == param_2) && (cVar1 = *(char *)(*param_3 + 4), cVar1 == cVar5)) {
      *puVar3 = *puVar3 | 0x20;
      if (*(char *)(param_1 + 8) != '\x01') {
        return 0;
      }
      *(char *)(param_1 + 8) = cVar1;
      FUN_8001f9cc/*0x8001f9cc*/(param_1,(uint)*(ushort *)(param_1 + 0x46) +
                              (uint)**(ushort **)(param_1 + 0x60));
      uVar2 = FUN_8004410c/*0x8004410c*/();
      FUN_800447e8/*0x800447e8*/(uVar2,_DAT_800658fc,0x14,*param_3 + 0x24);
    }
    FUN_8002239c/*0x8002239c*/(param_1,param_3);
    iVar6 = 1;
  }
  else {
    iVar6 = param_1;
    if (param_2 != 8) {
      cVar5 = '\x01';
      goto LAB_80101960;
    }
  }
  iVar6 = FUN_80022320/*0x80022320*/(iVar6,param_3);
  if (iVar6 == 0) {
    return 0;
  }
  func_0x80020844(param_1);
LAB_80101a38:
  puVar3 = *(uint **)(param_1 + 0x78);
  uVar4 = *puVar3 & 0xffffffdf;
  *puVar3 = uVar4;
  *(undefined1 *)(param_1 + 8) = 0;
  do {
    *(uint *)(param_1 + 0x78) = uVar4;
    *(char *)(uVar4 + 4) = (char)puVar3;
LAB_80101a68:
    uVar4 = *(uint *)(param_1 + 0x38);
    while( true ) {
      if (uVar4 == 0) {
        func_0x8001abd0(param_1);
        FUN_8001f9cc/*0x8001f9cc*/(param_1,0);
        return 0;
      }
      puVar3 = (uint *)0x3;
      if (*(short *)(uVar4 + 6) == 0) break;
      uVar4 = *(uint *)(uVar4 + 0x34);
    }
  } while( true );
}

