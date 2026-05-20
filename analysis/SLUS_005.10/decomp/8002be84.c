// addr: 0x8002be84  name: FUN_8002be84

void FUN_8002be84(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  uint *puVar3;
  int iVar4;
  char acStack_50 [64];
  
  FUN_80020890(param_1,300);
  *(undefined1 **)(param_1 + 100) = &LAB_8002bdd0;
  *(undefined2 *)(param_1 + 0xc) = 0;
  *(undefined2 *)(param_1 + 0xa6) = 0;
  *(undefined1 *)(param_1 + 0xb2) = 0;
  FUN_800441c8(*(undefined1 *)(param_1 + 0xd3));
  iVar4 = (int)*(char *)(param_1 + 5);
  if (iVar4 == 0) {
    cVar1 = FUN_8004410c();
    *(char *)(param_1 + 5) = cVar1;
    iVar4 = (int)cVar1;
  }
  FUN_8004483c(iVar4,uRam000005f8,0x22,param_1 + 0x24);
  if ((0 < *(short *)(param_1 + 6)) || (cRam00000015 == '\x03')) {
    uVar2 = FUN_8002b940(param_1);
    sprintf(acStack_50,"%s wrecked\nTotal it!",uVar2);
    iVar4 = 0;
    if (*(short *)(param_1 + 6) < 1) {
      iVar4 = *(short *)(param_1 + 6) + 3;
    }
    FUN_800129e8(iVar4,acStack_50);
  }
  puVar3 = (uint *)FUN_800407b4(DAT_800737d8,6,&DAT_80065710);
  *puVar3 = *puVar3 | 0x20000;
  uVar2 = FUN_8001b038(param_1,0x8101);
  FUN_8001b2fc(param_1,uVar2,puVar3);
  FUN_80020744(puVar3);
  FUN_800207c4(puVar3);
  return;
}

