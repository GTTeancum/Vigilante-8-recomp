// addr: 0x8001392c  name: FUN_8001392c

void FUN_8001392c(undefined4 param_1)

{
  bool bVar1;
  char cVar2;
  int iVar3;
  undefined4 uVar4;
  char *pcVar5;
  char *pcVar6;
  int iVar7;
  char acStack_110 [256];
  
  iVar3 = iRam000007d0;
  switch(uRam00000015) {
  case 0:
    if (iRam00000024 == 0) {
      sprintf(acStack_110,"\x01`  \a`\x05YOU LOSE!\n");
    }
    else {
      sprintf(acStack_110,&DAT_800100fc);
      FUN_800136c4(acStack_110,iRam000007d0);
      if ((iRam00000620 != 0) && (bRam0000061c < 0x20)) {
        iVar3 = FUN_80052544(acStack_110);
        sprintf(acStack_110 + iVar3,&DAT_80010110,
                (&PTR_s_Chassey_Blue_800567ec)[(uint)bRam0000061c * 2]);
      }
      strcat(acStack_110,&DAT_80010124);
      iVar3 = FUN_80052544(acStack_110);
      iVar7 = 0;
      pcVar6 = acStack_110 + iVar3;
      pcVar5 = &DAT_80065c08 + iRam00000620 * 0xe;
      do {
        cVar2 = *pcVar5;
        pcVar5 = pcVar5 + 1;
        iVar7 = iVar7 + 1;
        *pcVar6 = cVar2 + 'A';
        pcVar6 = pcVar6 + 1;
      } while (iVar7 < 0xe);
      *pcVar6 = '\0';
    }
    strcat(acStack_110,&DAT_800655c8);
    goto LAB_80013c80;
  case 1:
  case 2:
    bVar1 = iRam00000024 == 0;
    *(int *)(&DAT_80065978 + (uint)(iRam00000024 == 0) * 4) =
         *(int *)(&DAT_80065978 + (uint)(iRam00000024 == 0) * 4) + 1;
    if (bVar1) {
      sprintf(acStack_110,"\x01`  \a`\x05YOU LOSE!\n");
    }
    else {
      sprintf(acStack_110,"\x01 ` \a`\x05YOU WIN!\n");
      FUN_800136c4(acStack_110,iRam000007d0);
    }
    iVar3 = FUN_80052544(acStack_110);
    pcVar5 = acStack_110 + iVar3;
    pcVar6 = &DAT_8001017c;
    break;
  case 3:
    *(int *)(&DAT_80065978 + (uint)(*(short *)(iRam000007d0 + 0xc) == 0) * 4) =
         *(int *)(&DAT_80065978 + (uint)(*(short *)(iRam000007d0 + 0xc) == 0) * 4) + 1;
    uVar4 = 0x31;
    cRam0000067c = cRam0000067c + '\x01';
    if (*(short *)(iVar3 + 0xc) == 0) {
      uVar4 = 0x32;
    }
    sprintf(acStack_110,"\x01 ` \a`\x05PLAYER %c WINS!\n",uVar4);
    iVar3 = iRam000007d0;
    if (*(short *)(iRam000007d0 + 0xc) == 0) {
      iVar3 = iRam000007d4;
    }
    FUN_800136c4(acStack_110,iVar3);
    iVar3 = FUN_80052544(acStack_110);
    pcVar5 = acStack_110 + iVar3;
    pcVar6 = &DAT_800101c8;
    break;
  case 4:
    bVar1 = iRam00000024 == 0;
    *(int *)(&DAT_80065978 + (uint)(iRam00000024 == 0) * 4) =
         *(int *)(&DAT_80065978 + (uint)(iRam00000024 == 0) * 4) + 1;
    if (bVar1) {
      sprintf(acStack_110,"\x01`  \a`\x05PLAYER TEAM LOSES!\n");
    }
    else {
      sprintf(acStack_110,"\x01 ` \a`\x05PLAYER TEAM WINS!\n");
      *(char *)(iRam000007d0 + 0xba) =
           *(char *)(iRam000007d0 + 0xba) + *(char *)(iRam000007d4 + 0xba);
      *(char *)(iRam000007d0 + 0xbb) =
           *(char *)(iRam000007d0 + 0xbb) + *(char *)(iRam000007d4 + 0xbb);
      FUN_800136c4(acStack_110,iRam000007d0);
    }
    iVar3 = FUN_80052544(acStack_110);
    pcVar5 = acStack_110 + iVar3;
    pcVar6 = &DAT_80010238;
    break;
  default:
    goto switchD_80013964_default;
  }
  sprintf(pcVar5,pcVar6,uRam00000674,uRam00000678);
switchD_80013964_default:
  strcat(acStack_110,&DAT_80010274);
LAB_80013c80:
  uVar4 = FUN_80019458(param_1,acStack_110,0x40,0x40);
  FUN_80018efc(uVar4);
  return;
}

