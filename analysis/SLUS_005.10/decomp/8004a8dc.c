// addr: 0x8004a8dc  name: CD_datasync

undefined4 CD_datasync(int param_1)

{
  bool bVar1;
  int iVar2;
  undefined4 uVar3;
  
  iVar2 = VSync(-1);
  DAT_800a3258 = iVar2 + 0x3c0;
  DAT_800a325c = 0;
  DAT_800a3260 = "CD_datasync";
  while (iVar2 = VSync(-1), iVar2 <= DAT_800a3258) {
    iVar2 = DAT_800a325c + 1;
    bVar1 = 0x3c0000 < DAT_800a325c;
    DAT_800a325c = iVar2;
    if (bVar1) break;
    uVar3 = 0;
    if (((DMA_CDROM_CHCR & 0x1000000) == 0) || (uVar3 = 1, param_1 != 0)) {
      return uVar3;
    }
  }
  puts("CD timeout: ");
  printf("%s:(%s) Sync=%s, Ready=%s\n",DAT_800a3260,(&PTR_s_CdlSync_800600a4)[DAT_8006009d],
         (&PTR_s_NoIntr_80060124)[DAT_8006035c],(&PTR_s_NoIntr_80060124)[DAT_8006035d]);
  CD_flush();
  uVar3 = BIOS_OBJ_1440();
  return uVar3;
}

