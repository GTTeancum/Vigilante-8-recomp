// addr: 0x8010c690  name: FUN_8010c690

void FUN_8010c690(void)

{
  byte bVar1;
  byte bVar2;
  int in_v0;
  byte *pbVar3;
  
  pbVar3 = &DAT_80065950 + DAT_80065674;
  bVar1 = *pbVar3;
  if ((int)DAT_80065904 == 0x1f - in_v0) {
    *pbVar3 = bVar1 & ~(byte)(1 << ((int)DAT_80065904 & 0x1fU)) |
              (byte)(2 << ((int)DAT_80065904 & 0x1fU));
  }
  bVar2 = (byte)(1 << ((int)DAT_80065904 & 0x1fU));
  *pbVar3 = *pbVar3 | bVar2;
  DAT_80065920 = '\x1f' - bVar2;
  *pbVar3 = bVar1;
  return;
}

