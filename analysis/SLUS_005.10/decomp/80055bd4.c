// addr: 0x80055bd4  name: PadInitDirect

void PadInitDirect(uchar *param_1,uchar *param_2)

{
  int iVar1;
  undefined4 *puVar2;
  undefined *puVar3;
  undefined *puVar4;
  undefined *puVar5;
  int iVar6;
  undefined *puVar7;
  
  DAT_80065294 = 0;
  DAT_800652a8 = 0;
  _padInitDirSeq();
  puVar7 = &DAT_800a4d28;
  DAT_8006525c = PADPORTD_OBJ_1C8;
  DAT_80065260 = PADPORTD_OBJ_160;
  DAT_80065264 = PADPORTD_OBJ_2CC;
  DAT_80065268 = PADPORTD_OBJ_388;
  DAT_8006526c = PADPORTD_OBJ_604;
  DAT_80065270 = PADPORTD_OBJ_63C;
  DAT_80065290 = &DAT_800a4d28;
  DAT_80065280 = PADPORTD_OBJ_2BC;
  FUN_80044f64(&DAT_800a4d28,0x1e0);
  iVar6 = 0;
  puVar2 = &DAT_800a4d68;
  puVar5 = &DAT_800a4ce0;
  puVar4 = &DAT_800a4c98;
  DAT_800a4d58 = param_1;
  DAT_800a4e48 = param_2;
  do {
    puVar3 = puVar7 + 0x5d;
    puVar2[-0xd] = 0;
    puVar2[-0xc] = puVar7;
    *(undefined1 *)puVar2[-4] = 0xff;
    iVar1 = 5;
    *(undefined1 *)(puVar2[-4] + 1) = 0;
    puVar2[-1] = puVar4;
    *puVar2 = puVar5;
    do {
      *puVar3 = 0xff;
      iVar1 = iVar1 + -1;
      puVar3 = puVar3 + 1;
    } while (-1 < iVar1);
    puVar5 = puVar5 + 0x23;
    puVar4 = puVar4 + 0x23;
    iVar6 = iVar6 + 1;
    puVar2 = puVar2 + 0x3c;
    puVar7 = puVar7 + 0xf0;
  } while (iVar6 < 2);
  FUN_800541ac();
  DAT_80065294 = 1;
  return;
}

