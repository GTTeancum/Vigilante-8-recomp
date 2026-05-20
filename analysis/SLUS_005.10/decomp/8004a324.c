// addr: 0x8004a324  name: BIOS_OBJ_D70

undefined4 BIOS_OBJ_D70(void)

{
  undefined1 uVar1;
  bool bVar2;
  int in_v0;
  int iVar3;
  uint uVar4;
  undefined4 uVar5;
  undefined1 *puVar6;
  byte bVar7;
  byte *unaff_s2;
  int unaff_s3;
  undefined1 *unaff_s4;
  int unaff_s5;
  undefined1 *unaff_s6;
  
  while( true ) {
    if (in_v0 != 0) {
      return 0xffffffff;
    }
    iVar3 = CheckCallback();
    bVar7 = CDROM_REG0;
    if (iVar3 != 0) {
      bVar7 = CDROM_REG0 & 3;
      while( true ) {
        uVar4 = BIOS_OBJ_0();
        if (uVar4 == 0) break;
        if (((uVar4 & 4) != 0) && (DAT_80060080 != (code *)0x0)) {
          (*DAT_80060080)(*unaff_s4,&DAT_800a3248);
        }
        if (((uVar4 & 2) != 0) && (DAT_8006007c != (code *)0x0)) {
          (*DAT_8006007c)(*unaff_s2,&DAT_800a3240);
        }
      }
    }
    CDROM_REG0 = bVar7;
    if (*unaff_s2 != 0) break;
    iVar3 = VSync(-1);
    if ((DAT_800a3258 < iVar3) ||
       (iVar3 = DAT_800a325c + 1, bVar2 = 0x3c0000 < DAT_800a325c, DAT_800a325c = iVar3, bVar2)) {
      puts("CD timeout: ");
      printf("%s:(%s) Sync=%s, Ready=%s\n",DAT_800a3260,
             *(undefined4 *)((uint)DAT_8006009d * 4 + unaff_s5),
             *(undefined4 *)((uint)*unaff_s2 * 4 + unaff_s3));
      CD_flush();
      uVar5 = BIOS_OBJ_D70();
      return uVar5;
    }
    in_v0 = 0;
  }
  puVar6 = &DAT_800a3240;
  iVar3 = 7;
  if (unaff_s6 != (undefined1 *)0x0) {
    do {
      uVar1 = *puVar6;
      puVar6 = puVar6 + 1;
      iVar3 = iVar3 + -1;
      *unaff_s6 = uVar1;
      unaff_s6 = unaff_s6 + 1;
    } while (iVar3 != -1);
  }
  uVar5 = 0;
  if (DAT_8006035c == '\x05') {
    uVar5 = 0xffffffff;
  }
  return uVar5;
}

