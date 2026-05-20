// addr: 0x8004a354  name: BIOS_OBJ_DA0

undefined4 BIOS_OBJ_DA0(void)

{
  undefined1 uVar1;
  bool bVar2;
  byte bVar3;
  uint uVar4;
  undefined4 uVar5;
  int iVar6;
  undefined1 *puVar7;
  byte unaff_s1;
  byte *unaff_s2;
  int unaff_s3;
  undefined1 *unaff_s4;
  int unaff_s5;
  undefined1 *unaff_s6;
  
  do {
    while( true ) {
      uVar4 = BIOS_OBJ_0();
      bVar3 = unaff_s1;
      if (uVar4 == 0) break;
      if (((uVar4 & 4) != 0) && (DAT_80060080 != (code *)0x0)) {
        (*DAT_80060080)(*unaff_s4,&DAT_800a3248);
      }
      if (((uVar4 & 2) != 0) && (DAT_8006007c != (code *)0x0)) {
        (*DAT_8006007c)(*unaff_s2,&DAT_800a3240);
      }
    }
    do {
      CDROM_REG0 = bVar3;
      if (*unaff_s2 != 0) {
        puVar7 = &DAT_800a3240;
        iVar6 = 7;
        if (unaff_s6 != (undefined1 *)0x0) {
          do {
            uVar1 = *puVar7;
            puVar7 = puVar7 + 1;
            iVar6 = iVar6 + -1;
            *unaff_s6 = uVar1;
            unaff_s6 = unaff_s6 + 1;
          } while (iVar6 != -1);
        }
        uVar5 = 0;
        if (DAT_8006035c == '\x05') {
          uVar5 = 0xffffffff;
        }
        return uVar5;
      }
      iVar6 = VSync(-1);
      if ((DAT_800a3258 < iVar6) ||
         (iVar6 = DAT_800a325c + 1, bVar2 = 0x3c0000 < DAT_800a325c, DAT_800a325c = iVar6, bVar2)) {
        puts("CD timeout: ");
        printf("%s:(%s) Sync=%s, Ready=%s\n",DAT_800a3260,
               *(undefined4 *)((uint)DAT_8006009d * 4 + unaff_s5),
               *(undefined4 *)((uint)*unaff_s2 * 4 + unaff_s3));
        CD_flush();
        uVar5 = BIOS_OBJ_D70();
        return uVar5;
      }
      iVar6 = CheckCallback();
      bVar3 = CDROM_REG0;
    } while (iVar6 == 0);
    unaff_s1 = CDROM_REG0 & 3;
  } while( true );
}

