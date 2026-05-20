// addr: 0x800136c4  name: FUN_800136c4

void FUN_800136c4(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  undefined *puVar5;
  uint uVar6;
  int iVar7;
  
  iVar2 = (uint)*(byte *)(param_2 + 0xba) * 300 + cRam0000067c * 0xa8c +
          (uint)*(byte *)(param_2 + 0xbb) * 600;
  iVar7 = iRam0000000c;
  if (iVar2 < iRam0000000c) {
    iVar7 = iVar2;
  }
  if (cRam00000015 == '\0') {
    iVar7 = (iVar7 * 0x55) / iRam0000000c;
    if (iRam00000620 != 0) {
      iVar7 = iVar7 + 0xf;
    }
  }
  else {
    iVar7 = (iVar7 * 100) / iRam0000000c;
  }
  iVar2 = FUN_80052544(param_1);
  uVar3 = (uint)*(byte *)(param_2 + 0xba);
  iVar4 = (int)cRam0000067c;
  uVar6 = (uint)*(byte *)(param_2 + 0xbb);
  sprintf((char *)(param_1 + iVar2),&DAT_80010084,iRam0000000c / 0xe10,
          iRam0000000c / 0x3c + (iRam0000000c / 0xe10) * -0x3c,uVar3,uVar6,iVar4);
  if (cRam00000015 == '\0') {
    iVar2 = FUN_80052544(param_1);
    puVar5 = &DAT_80065468;
    if (iRam00000620 != 0) {
      puVar5 = &DAT_80065464;
    }
    sprintf((char *)(param_1 + iVar2),&DAT_800100cc,puVar5);
  }
  iVar2 = FUN_80052544(param_1);
  if ((*(char *)(param_2 + 0xd0) == '\f') || (cRam00000015 == '\x04')) {
    puVar5 = &DAT_80065344;
  }
  else {
    iVar1 = (iVar7 < 0x1a ^ 1) + (iVar7 < 0x29 ^ 1) + (iVar7 < 0x3d ^ 1) + (iVar7 < 0x4c ^ 1);
    if (0x5a < iVar7) {
      iVar1 = iVar1 + 1;
    }
    iVar1 = iVar1 * 4;
    if (5 < *(byte *)(param_2 + 0xd0)) {
      iVar1 = iVar1 + 0x18;
    }
    puVar5 = *(undefined **)((int)&PTR_s_Civilian_80056884 + iVar1);
  }
  sprintf((char *)(param_1 + iVar2),&DAT_800100e4,iVar7,puVar5,uVar3,uVar6,iVar4);
  return;
}

