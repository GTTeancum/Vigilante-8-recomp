// addr: 0x801014d0  name: FUN_801014d0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_801014d0(uint *param_1,undefined4 param_2,uint *param_3)

{
  char cVar1;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  uint *puVar5;
  uint uVar6;
  
  switch(param_2) {
  case 0:
    *(short *)(param_1[0xe] + 0x42) = *(short *)(param_1[0xe] + 0x42) + 0x22;
    if (param_3 == (uint *)0x0) {
      return 0;
    }
    FUN_8001d708/*0x8001d708*/(param_1[0xe]);
  case 3:
    puVar5 = (uint *)param_3[3];
    if ((((char)puVar5[1] == '\x03') && (*(char *)(*param_3 + 4) == '\x02')) &&
       ((*param_1 & 0x80) != 0)) {
      *puVar5 = *puVar5 | 0x20;
      FUN_8001f9cc/*0x8001f9cc*/(param_1,(uint)*(ushort *)((int)param_1 + 0x46) +
                              (uint)*(ushort *)param_1[0x18]);
      uVar2 = FUN_8004410c/*0x8004410c*/();
      FUN_800447e8/*0x800447e8*/(uVar2,*(undefined4 *)(param_1[0x16] + 8),2,param_1 + 0x12);
      puVar5 = (uint *)FUN_8001ffd4/*0x8001ffd4*/(0x80065a18,0x71);
      if (puVar5 != (uint *)0x0) {
        uVar3 = *param_3;
        *(undefined1 *)(puVar5 + 2) = 2;
        uVar6 = *(uint *)(uVar3 + 0xe4);
        if (uVar6 == 0) {
          uVar6 = uVar3;
        }
        puVar5[0x29] = uVar6;
        *puVar5 = *puVar5 & 0xffffffdd;
        func_0x80020844(puVar5);
        if ((*puVar5 & 0x80) == 0) {
          cVar1 = FUN_8004410c/*0x8004410c*/();
          *(char *)((int)puVar5 + 5) = cVar1;
          FUN_800443c8/*0x800443c8*/((int)cVar1,*(undefined4 *)(puVar5[0x16] + 8),1,0);
          FUN_80020744/*0x80020744*/(puVar5);
        }
      }
      *(undefined1 *)(param_1 + 2) = 1;
    }
    FUN_8002239c/*0x8002239c*/(param_1,param_3);
switchD_80101510_caseD_8:
    iVar4 = FUN_80022320/*0x80022320*/(param_1,param_3);
    if (iVar4 != 0) {
      FUN_80020778/*0x80020778*/(param_1);
      func_0x80020844(param_1);
switchD_80101510_caseD_2:
      puVar5 = param_1;
      if ((*param_1 & 0x80) != 0) {
        **(uint **)(param_1[0xe] + 0x34) = **(uint **)(param_1[0xe] + 0x34) | 0x20;
        *(undefined1 *)(param_1 + 2) = 0;
        FUN_80020778/*0x80020778*/(param_1);
        FUN_8001ac08/*0x8001ac08*/(param_1);
        FUN_8001f9cc/*0x8001f9cc*/(param_1,(int)_DAT_800659d0);
        puVar5 = (uint *)0x1;
      }
      **(uint **)(param_1[0xe] + 0x34) = **(uint **)(param_1[0xe] + 0x34) & 0xffffffdf;
      FUN_80020744/*0x80020744*/(puVar5);
      **(ushort **)(param_1[0xe] + 0x30) = **(ushort **)(param_1[0xe] + 0x30) ^ 5;
      FUN_80020890/*0x80020890*/(param_1,0x708);
      goto switchD_80101510_caseD_1;
    }
    break;
  case 1:
switchD_80101510_caseD_1:
    *(undefined1 *)(*(int *)(param_1[0xe] + 0x34) + 4) = 3;
    FUN_80020890/*0x80020890*/(param_1,0x708);
    FUN_8001ac08/*0x8001ac08*/(param_1);
    FUN_8001f9cc/*0x8001f9cc*/(param_1,(int)_DAT_800659d0);
    break;
  case 2:
    goto switchD_80101510_caseD_2;
  default:
    break;
  case 8:
    goto switchD_80101510_caseD_8;
  }
  return 0;
}

