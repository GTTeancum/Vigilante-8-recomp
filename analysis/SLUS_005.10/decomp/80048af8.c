// addr: 0x80048af8  name: CdGetToc2

undefined4 CdGetToc2(undefined4 param_1,byte *param_2)

{
  byte *pbVar1;
  byte bVar2;
  CdlCB func;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  uint uVar6;
  uint uVar7;
  u_char local_30 [8];
  u_char uStack_28;
  byte local_27;
  byte local_26;
  
  local_30[0] = '\x01';
  func = CdSyncCallback((CdlCB)0x0);
  iVar3 = CdControlB('\x13',(u_char *)0x0,&uStack_28);
  if (iVar3 != 0) {
    uVar6 = (uint)(local_27 >> 4) * 10 + (local_27 & 0xf);
    uVar7 = (uint)(local_26 >> 4) * 10 + (local_26 & 0xf);
    if (1 < DAT_80060088) {
      printf("track=%d,%d\n",uVar6,uVar7);
    }
    local_30[0] = '\0';
    iVar3 = CdControlB('\x14',local_30,&uStack_28);
    if (iVar3 != 0) {
      *param_2 = local_27;
      param_2[2] = 0;
      param_2[1] = local_26;
      iVar3 = 1;
      pbVar1 = param_2;
      if (uVar6 <= uVar7) {
        do {
          local_30[0] = (char)uVar6 + (char)((int)uVar6 / 10) * '\x06';
          iVar4 = CdControlB('\x14',local_30,&uStack_28);
          iVar3 = iVar3 + 1;
          if (iVar4 == 0) goto TOC_OBJ_1F4;
          pbVar1[4] = local_27;
          uVar6 = uVar6 + 1;
          pbVar1[6] = 0;
          pbVar1[5] = local_26;
          pbVar1 = pbVar1 + 4;
        } while ((int)uVar6 <= (int)uVar7);
      }
      if ((1 < DAT_80060088) && (iVar4 = 0, -1 < iVar3 + -1)) {
        do {
          bVar2 = *param_2;
          pbVar1 = param_2 + 1;
          param_2 = param_2 + 4;
          iVar4 = iVar4 + 1;
          printf("CdGetToc2: %02x:%02x:00\n",(uint)bVar2,(uint)*pbVar1);
        } while (iVar4 <= iVar3 + -1);
      }
      CdSyncCallback(func);
      uVar5 = TOC_OBJ_220();
      return uVar5;
    }
  }
TOC_OBJ_1F4:
  if (DAT_80060088 != 0) {
    printf("CdGetToc2: error\n");
  }
  CdSyncCallback(func);
  return 0;
}

