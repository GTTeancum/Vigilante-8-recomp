// addr: 0x80100e54  name: FUN_80100e54

undefined4 FUN_80100e54(int param_1,int param_2,int *param_3)

{
  undefined2 uVar1;
  int iVar2;
  undefined4 uVar3;
  uint *puVar4;
  uint *puVar5;
  short *psVar6;
  int local_50;
  int local_4c;
  int local_48;
  undefined1 auStack_40 [20];
  uint local_2c;
  uint local_28;
  uint local_24;
  
  if ((param_2 == 3) || (param_2 != 8)) {
    psVar6 = (short *)param_3[1];
    if ((*psVar6 == 1) &&
       ((psVar6[1] != 0 && (puVar4 = (uint *)*param_3, (char)puVar4[1] == '\x02')))) {
      puVar5 = (uint *)puVar4[0x38];
      iVar2 = FUN_8001b038/*0x8001b038*/(param_1,0x8000);
      uVar3 = FUN_8004410c/*0x8004410c*/();
      FUN_800447e8/*0x800447e8*/(uVar3,*(undefined4 *)(*(int *)(param_1 + 0x58) + 8),5,puVar4 + 9);
      FUN_80044574/*0x80044574*/((int)*(char *)((int)puVar4 + 5),0);
      puVar4[0x19] = (uint)FUN_80100c6c;
      puVar4[0x20] = 0;
      *puVar4 = *puVar4 | 0x3000022;
      puVar4[0x21] = 0x1c980;
      puVar4[0x22] = 0;
      if (puVar5 != (uint *)0x0) {
        *(undefined2 *)(puVar5 + 0x21) = 0;
        *puVar5 = *puVar5 | 0xc0000;
        *(undefined2 *)((int)puVar5 + 0x86) = 0xfe37;
        *(undefined2 *)(puVar5 + 0x22) = 0xf415;
      }
      local_50 = (*(int *)(psVar6 + 2) + *(int *)(psVar6 + 8)) / 2;
      local_4c = *(int *)(psVar6 + 10);
      local_48 = (*(int *)(psVar6 + 6) + *(int *)(psVar6 + 0xc)) / 2;
      GTE_RotateLongMatTrans/*0x80043408*/(param_1 + 0x10,&local_50,&local_50);
      puVar4[0x20] = (local_50 - puVar4[9]) * 2;
      puVar4[0x21] = (local_4c - puVar4[10]) * 2;
      puVar4[0x22] = (local_48 - puVar4[0xb]) * 2;
      FUN_8001d68c/*0x8001d68c*/(auStack_40,param_1,iVar2);
      puVar4[0x12] = local_2c;
      puVar4[0x13] = local_28;
      puVar4[0x14] = local_24;
      uVar1 = *(undefined2 *)(iVar2 + 0x14);
      puVar4[0x10] = *(uint *)(iVar2 + 0x10);
      *(undefined2 *)(puVar4 + 0x11) = uVar1;
      FUN_80020890/*0x80020890*/(puVar4,0x40);
      param_3 = (int *)0x1;
    }
    if (*(char *)(*param_3 + 4) != '\a') {
      return 0;
    }
    param_3 = (int *)(uint)*(ushort *)(*param_3 + 0xc);
  }
  FUN_80022320/*0x80022320*/(param_1,param_3);
  return 0;
}

