// addr: 0x8010ec60  name: FUN_8010ec60

int FUN_8010ec60(byte *param_1)

{
  bool bVar1;
  int iVar2;
  uint uVar3;
  byte bVar4;
  byte *pbVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  
  iVar8 = 1;
  uVar6 = 10;
  iVar2 = 0;
  if (param_1 != (byte *)0x0) {
    do {
      pbVar5 = param_1;
      param_1 = pbVar5 + 1;
    } while ((*(byte *)(*pbVar5 + 0x80065175) & 8) != 0);
    bVar4 = *pbVar5;
    if (bVar4 == 0x2d) {
      do {
        pbVar5 = pbVar5 + 1;
        iVar8 = -iVar8;
      } while (*pbVar5 == 0x2d);
      bVar4 = *pbVar5;
    }
    uVar3 = 0x58;
    if (bVar4 == 0x30) {
      bVar4 = pbVar5[1];
      uVar3 = (uint)((char)bVar4 < 'Y');
      if (((bVar4 == 0x58) ||
          (((bVar1 = uVar3 != 0, uVar3 = 0x42, bVar1 && (bVar4 == 0x42)) ||
           (uVar3 = 0x78, bVar4 == 0x62)))) || (uVar6 = 8, bVar4 == 0x78)) {
        uVar6 = 2;
      }
    }
    uVar7 = 0;
    if (uVar3 < uVar6) {
      uVar7 = uVar3;
    }
    iVar2 = uVar7 * iVar8;
  }
  return iVar2;
}

