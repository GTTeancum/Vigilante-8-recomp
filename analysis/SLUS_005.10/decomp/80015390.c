// addr: 0x80015390  name: FUN_80015390

void FUN_80015390(void)

{
  undefined4 uVar1;
  uint uVar2;
  char *pcVar3;
  undefined4 *puVar4;
  int iVar5;
  
  puVar4 = EPC;
  uVar2 = Cause;
  uVar1 = BadVAddr;
  FUN_80015164(0);
  switch(uVar2 & 0x3c) {
  case 0x10:
    pcVar3 = "Read Address Error (%08x)";
    goto LAB_80015414;
  case 0x14:
    pcVar3 = "Write Address Error (%08x)";
LAB_80015414:
    sprintf(&DAT_8006f5c8,pcVar3,uVar1);
    goto LAB_800154a0;
  case 0x18:
    pcVar3 = "Instruction Bus Error";
    break;
  case 0x1c:
    pcVar3 = "Data Bus Error";
    break;
  default:
    pcVar3 = "Unknown Exception";
    break;
  case 0x28:
    pcVar3 = "Illegal Instruction";
    break;
  case 0x2c:
    pcVar3 = "CoProcessor Unusable";
    break;
  case 0x30:
    pcVar3 = "Overflow";
  }
  sprintf(&DAT_8006f5c8,pcVar3);
LAB_800154a0:
  FUN_80015288(&DAT_8006f5c8);
  iVar5 = 0;
  do {
    iVar5 = iVar5 + 1;
    sprintf(&DAT_8006f5c8,"%08x: %08x",puVar4,*puVar4);
    FUN_80015288(&DAT_8006f5c8);
    puVar4 = puVar4 + 1;
  } while (iVar5 < 4);
  do {
                    /* WARNING: Do nothing block with infinite loop */
  } while( true );
}

