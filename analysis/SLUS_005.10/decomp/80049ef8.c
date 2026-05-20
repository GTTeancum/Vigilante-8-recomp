// addr: 0x80049ef8  name: BIOS_OBJ_944

undefined4 BIOS_OBJ_944(void)

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
  undefined1 *unaff_s3;
  char *unaff_s4;
  int unaff_s5;
  undefined1 *unaff_s6;
  int unaff_s7;
  int unaff_s8;
  
  do {
    while( true ) {
      uVar4 = BIOS_OBJ_0();
      bVar3 = unaff_s1;
      if (uVar4 == 0) break;
      if (((uVar4 & 4) != 0) && (DAT_80060080 != (code *)0x0)) {
        (*DAT_80060080)(*unaff_s6,&DAT_800a3248);
      }
      if (((uVar4 & 2) != 0) && (DAT_8006007c != (code *)0x0)) {
        (*DAT_8006007c)(*unaff_s2,&DAT_800a3240);
      }
    }
    do {
      CDROM_REG0 = bVar3;
      if (*unaff_s4 != '\0') {
        unaff_s2[2] = 0;
        puVar7 = &DAT_800a3250;
        if (unaff_s3 != (undefined1 *)0x0) {
          iVar6 = 7;
          do {
            uVar1 = *puVar7;
            puVar7 = puVar7 + 1;
            iVar6 = iVar6 + -1;
            *unaff_s3 = uVar1;
            unaff_s3 = unaff_s3 + 1;
          } while (iVar6 != -1);
          uVar5 = BIOS_OBJ_A74();
          return uVar5;
        }
        goto BIOS_OBJ_A64;
      }
      if (unaff_s4[-1] != '\0') {
        unaff_s2[1] = 0;
        puVar7 = &DAT_800a3248;
        iVar6 = 7;
        if (unaff_s3 != (undefined1 *)0x0) {
          do {
            uVar1 = *puVar7;
            puVar7 = puVar7 + 1;
            iVar6 = iVar6 + -1;
            *unaff_s3 = uVar1;
            unaff_s3 = unaff_s3 + 1;
          } while (iVar6 != -1);
        }
BIOS_OBJ_A64:
        uVar5 = BIOS_OBJ_A74();
        return uVar5;
      }
      if (unaff_s7 != 0) {
        return 0;
      }
      iVar6 = VSync(-1);
      if ((DAT_800a3258 < iVar6) ||
         (iVar6 = DAT_800a325c + 1, bVar2 = 0x3c0000 < DAT_800a325c, DAT_800a325c = iVar6, bVar2)) {
        puts("CD timeout: ");
        printf("%s:(%s) Sync=%s, Ready=%s\n",DAT_800a3260,
               *(undefined4 *)((uint)DAT_8006009d * 4 + unaff_s8),
               *(undefined4 *)((uint)*unaff_s2 * 4 + unaff_s5));
        CD_flush();
        uVar5 = BIOS_OBJ_914();
        return uVar5;
      }
      iVar6 = CheckCallback();
      bVar3 = CDROM_REG0;
    } while (iVar6 == 0);
    unaff_s1 = CDROM_REG0 & 3;
  } while( true );
}

