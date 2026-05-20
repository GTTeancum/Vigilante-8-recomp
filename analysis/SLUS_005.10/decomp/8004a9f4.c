// addr: 0x8004a9f4  name: BIOS_OBJ_1440

undefined4 BIOS_OBJ_1440(void)

{
  bool bVar1;
  int iVar2;
  int in_v0;
  undefined4 uVar3;
  int unaff_s0;
  byte *unaff_s1;
  int unaff_s2;
  int unaff_s3;
  int unaff_s4;
  
  while (((uVar3 = 0xffffffff, in_v0 == 0 && (uVar3 = 0, (DMA_CDROM_CHCR & 0x1000000) != 0)) &&
         (uVar3 = 1, unaff_s2 == 0))) {
    iVar2 = VSync(-1);
    if ((DAT_800a3258 < iVar2) ||
       (iVar2 = DAT_800a325c + 1, bVar1 = unaff_s4 < DAT_800a325c, DAT_800a325c = iVar2, bVar1)) {
      puts("CD timeout: ");
      printf("%s:(%s) Sync=%s, Ready=%s\n",DAT_800a3260,
             *(undefined4 *)((uint)DAT_8006009d * 4 + unaff_s3),
             *(undefined4 *)((uint)*unaff_s1 * 4 + unaff_s0));
      CD_flush();
      uVar3 = BIOS_OBJ_1440();
      return uVar3;
    }
    in_v0 = 0;
  }
  return uVar3;
}

