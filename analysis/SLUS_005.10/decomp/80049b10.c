// addr: 0x80049b10  name: CD_sync

undefined4 CD_sync(int param_1,undefined1 *param_2)

{
  undefined1 uVar1;
  bool bVar2;
  int iVar3;
  undefined4 uVar4;
  uint uVar5;
  undefined1 *puVar6;
  byte bVar7;
  
  iVar3 = VSync(-1);
  DAT_800a3258 = iVar3 + 0x3c0;
  DAT_800a325c = 0;
  DAT_800a3260 = "CD_sync";
  while( true ) {
    iVar3 = VSync(-1);
    if ((DAT_800a3258 < iVar3) ||
       (iVar3 = DAT_800a325c + 1, bVar2 = 0x3c0000 < DAT_800a325c, DAT_800a325c = iVar3, bVar2)) {
      puts("CD timeout: ");
      printf("%s:(%s) Sync=%s, Ready=%s\n",DAT_800a3260,(&PTR_s_CdlSync_800600a4)[DAT_8006009d],
             (&PTR_s_NoIntr_80060124)[DAT_8006035c],(&PTR_s_NoIntr_80060124)[DAT_8006035d]);
      CD_flush();
      uVar4 = BIOS_OBJ_694();
      return uVar4;
    }
    iVar3 = CheckCallback();
    bVar7 = CDROM_REG0;
    if (iVar3 != 0) {
      bVar7 = CDROM_REG0 & 3;
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
    CDROM_REG0 = bVar7;
    if ((DAT_8006035c == 2) || (DAT_8006035c == 5)) break;
    if (param_1 != 0) {
      return 0;
    }
  }
  DAT_8006035c = 2;
  puVar6 = &DAT_800a3240;
  iVar3 = 7;
  if (param_2 != (undefined1 *)0x0) {
    do {
      uVar1 = *puVar6;
      puVar6 = puVar6 + 1;
      iVar3 = iVar3 + -1;
      *param_2 = uVar1;
      param_2 = param_2 + 1;
    } while (iVar3 != -1);
  }
  uVar4 = BIOS_OBJ_7AC();
  return uVar4;
}

