// addr: 0x801014d0  name: FUN_801014d0

/* WARNING: Type propagation algorithm not settling */

undefined8 FUN_801014d0(undefined1 *param_1)

{
  undefined1 uVar1;
  byte *pbVar2;
  byte *pbVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  
  uVar6 = 0;
  uVar7 = 0;
  pbVar2 = &DAT_80065950;
  iVar5 = 0;
  do {
    iVar4 = iVar5 + 1;
    uVar7 = uVar7 << 5 | uVar6 >> 0x1b;
    uVar6 = uVar6 << 5 | (uint)*pbVar2;
    pbVar2 = (byte *)(iVar5 + -0x7ff9a6af);
    iVar5 = iVar4;
  } while (iVar4 < 0xc);
  iVar5 = 0;
  pbVar3 = (byte *)0x0;
  do {
    uVar6 = uVar7;
    pbVar3 = pbVar3 + (int)pbVar2;
    iVar5 = iVar5 + 1;
    pbVar2 = (byte *)(uint)(iVar5 < 0xd);
    uVar7 = 0;
  } while (pbVar2 != (byte *)0x0);
  V8_SeedRng/*0x8001714c*/(0x31415926,uVar6,0x1a,0);
  iVar5 = (int)pbVar3 % 0x1a;
  while (iVar5 = iVar5 + -1, iVar5 != -1) {
    FUN_80017160/*0x80017160*/();
  }
  uVar7 = func_0x8001719c();
  func_0x8001719c();
  uVar1 = 0;
  *param_1 = (char)((int)pbVar3 % 0x1a);
  iVar5 = 1;
  do {
    param_1[iVar5] = uVar1;
    iVar5 = iVar5 + 1;
    uVar1 = 1;
  } while (iVar5 < 0xe);
  return CONCAT44(uVar7 >> 3,param_1);
}

