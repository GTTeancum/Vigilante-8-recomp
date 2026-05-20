// addr: 0x8003c538  name: FUN_8003c538

undefined4 FUN_8003c538(uint *param_1,uint param_2)

{
  undefined4 uVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  
  if ((*param_1 & 0x10000) == 0) {
    uVar1 = 1;
  }
  else {
    if ((param_2 < 0x80000001) && (iVar2 = FUN_8003c288(param_1,param_1[0x20]), iVar2 == 0)) {
      return 0;
    }
    uVar1 = FUN_8004410c();
    iVar2 = FUN_8001d624(param_1);
    FUN_8004483c(uVar1,uRam000005f8,0x2c,iVar2 + 0x14);
    uVar3 = param_1[0x20];
    *param_1 = *param_1 & 0xfffeffff;
    uVar4 = *(uint *)(uVar3 + 8);
    uVar5 = *(uint *)(uVar3 + 0xc);
    param_1[0x12] = *(uint *)(uVar3 + 4);
    param_1[0x13] = uVar4;
    param_1[0x14] = uVar5;
    param_1[9] = param_1[0x12];
    param_1[10] = param_1[0x13];
    param_1[0xb] = param_1[0x14];
    FUN_80020778();
    uVar1 = 0;
  }
  return uVar1;
}

