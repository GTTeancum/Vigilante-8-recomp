// addr: 0x8004a058  name: CD_cw

undefined4 CD_cw(byte param_1,byte *param_2,undefined1 *param_3,int param_4)

{
  undefined1 uVar1;
  bool bVar2;
  undefined4 uVar3;
  byte *pbVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  undefined1 *puVar8;
  byte bVar9;
  
  if (1 < DAT_80060088) {
    printf("%s...\n",(&PTR_s_CdlSync_800600a4)[param_1]);
  }
  if ((*(int *)(&DAT_800602c4 + (uint)param_1 * 4) == 0) || (param_2 != (byte *)0x0)) {
    CD_sync(0,0);
    if (param_1 == 2) {
      iVar6 = 0;
      pbVar4 = param_2;
      do {
        (&DAT_80060098)[iVar6] = *pbVar4;
        iVar6 = iVar6 + 1;
        pbVar4 = param_2 + iVar6;
      } while (iVar6 < 4);
    }
    if (param_1 == 0xe) {
      DAT_8006009c = *param_2;
    }
    iVar6 = (uint)param_1 * 4;
    DAT_8006035c = 0;
    if (*(int *)(&DAT_800601c4 + iVar6) != 0) {
      DAT_8006035d = 0;
    }
    CDROM_REG0 = 0;
    iVar7 = 0;
    pbVar4 = param_2;
    if (0 < *(int *)(&DAT_800602c4 + iVar6)) {
      do {
        CDROM_REG2 = *pbVar4;
        iVar7 = iVar7 + 1;
        pbVar4 = param_2 + iVar7;
      } while (iVar7 < *(int *)(&DAT_800602c4 + iVar6));
    }
    uVar3 = 0;
    DAT_8006009d = param_1;
    CDROM_REG1 = param_1;
    if (param_4 == 0) {
      iVar6 = VSync(-1);
      DAT_800a3258 = iVar6 + 0x3c0;
      DAT_800a325c = 0;
      DAT_800a3260 = "CD_cw";
      bVar9 = CDROM_REG0;
      while (CDROM_REG0 = bVar9, DAT_8006035c == 0) {
        iVar6 = VSync(-1);
        if ((DAT_800a3258 < iVar6) ||
           (iVar6 = DAT_800a325c + 1, bVar2 = 0x3c0000 < DAT_800a325c, DAT_800a325c = iVar6, bVar2))
        {
          puts("CD timeout: ");
          printf("%s:(%s) Sync=%s, Ready=%s\n",DAT_800a3260,(&PTR_s_CdlSync_800600a4)[DAT_8006009d],
                 (&PTR_s_NoIntr_80060124)[DAT_8006035c],(&PTR_s_NoIntr_80060124)[DAT_8006035d]);
          CD_flush();
          uVar3 = BIOS_OBJ_D70();
          return uVar3;
        }
        iVar6 = CheckCallback();
        bVar9 = CDROM_REG0;
        if (iVar6 != 0) {
          bVar9 = CDROM_REG0 & 3;
          while( true ) {
            uVar5 = BIOS_OBJ_0();
            if (uVar5 == 0) break;
            if (((uVar5 & 4) != 0) && (DAT_80060080 != (code *)0x0)) {
              (*DAT_80060080)(DAT_8006035d,&DAT_800a3248);
            }
            if (((uVar5 & 2) != 0) && (DAT_8006007c != (code *)0x0)) {
              (*DAT_8006007c)(DAT_8006035c,&DAT_800a3240);
            }
          }
        }
      }
      puVar8 = &DAT_800a3240;
      iVar6 = 7;
      if (param_3 != (undefined1 *)0x0) {
        do {
          uVar1 = *puVar8;
          puVar8 = puVar8 + 1;
          iVar6 = iVar6 + -1;
          *param_3 = uVar1;
          param_3 = param_3 + 1;
        } while (iVar6 != -1);
      }
      uVar3 = 0;
      if (DAT_8006035c == 5) {
        uVar3 = 0xffffffff;
      }
    }
  }
  else {
    uVar3 = 0xfffffffe;
    if (0 < DAT_80060088) {
      printf("%s: no param\n",(&PTR_s_CdlSync_800600a4)[param_1]);
      uVar3 = BIOS_OBJ_E88();
      return uVar3;
    }
  }
  return uVar3;
}

