// addr: 0x8005532c  name: PADCMD_OBJ_698

undefined4 PADCMD_OBJ_698(int param_1)

{
  byte bVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  byte *pbVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;
  
  uVar7 = 0;
  if (*(char *)(param_1 + 0xe9) != '\0') {
    iVar9 = 0;
    do {
      pbVar5 = *(byte **)(param_1 + 0x20);
      iVar6 = 0;
      iVar3 = 5;
      do {
        bVar1 = *pbVar5;
        pbVar5 = pbVar5 + 1;
        if (bVar1 == uVar7) {
          iVar6 = iVar6 + 1;
        }
        iVar3 = iVar3 + -1;
      } while (-1 < iVar3);
      uVar8 = (uint)*(byte *)(iVar9 + *(int *)(param_1 + 4) + 2);
      pbVar5 = *(byte **)(param_1 + 0x20);
      iVar4 = 0;
      iVar3 = param_1;
      if (uVar8 == 0) {
        uVar8 = 1;
      }
      do {
        bVar1 = *pbVar5;
        pbVar5 = pbVar5 + 1;
        if (bVar1 == uVar7) {
          if (iVar6 < (int)uVar8) {
            *(undefined1 *)(iVar3 + 0x5d) = 0xff;
            uVar2 = PADCMD_OBJ_72C();
            return uVar2;
          }
          *(char *)(iVar3 + 0x5d) = (char)uVar7;
        }
        iVar4 = iVar4 + 1;
        iVar3 = iVar3 + 1;
      } while (iVar4 < 6);
      uVar7 = uVar7 + 1;
      iVar9 = iVar9 + 5;
    } while ((int)uVar7 < (int)(uint)*(byte *)(param_1 + 0xe9));
  }
  *(undefined1 *)(param_1 + 0x46) = 0xfe;
  return 0;
}

