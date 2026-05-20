// addr: 0x8002c018  name: FUN_8002c018

void FUN_8002c018(uint *param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  uint uVar5;
  
  if ((*param_1 & 0x20000) == 0) {
    uVar5 = param_1[0x17];
    iVar1 = FUN_8001ac44(DAT_800737dc,0x26,0x80,8);
    *(undefined1 **)(iVar1 + 100) = &LAB_8002bfb8;
    *(int *)(iVar1 + 0x24) = (*(int *)(uVar5 + 4) + *(int *)(uVar5 + 0x10)) / 2;
    *(undefined4 *)(iVar1 + 0x28) = *(undefined4 *)(uVar5 + 0x14);
    iVar2 = *(int *)(uVar5 + 0xc);
    iVar4 = *(int *)(uVar5 + 0x18);
    *(undefined2 *)(iVar1 + 0x1e) = 0;
    *(undefined2 *)(iVar1 + 0x1c) = 0;
    *(undefined2 *)(iVar1 + 0x1a) = 0;
    *(undefined2 *)(iVar1 + 0x16) = 0;
    *(undefined2 *)(iVar1 + 0x14) = 0;
    *(undefined2 *)(iVar1 + 0x12) = 0;
    *(int *)(iVar1 + 0x2c) = (iVar2 + iVar4) / 2;
    iVar2 = *(int *)(uVar5 + 0x10) - *(int *)(uVar5 + 4);
    if (iVar2 < 0) {
      iVar2 = iVar2 + 7;
    }
    *(short *)(iVar1 + 0x10) = (short)(iVar2 >> 3);
    iVar2 = *(int *)(uVar5 + 0x14) - *(int *)(uVar5 + 8);
    if (iVar2 < 0) {
      iVar2 = iVar2 + 3;
    }
    *(short *)(iVar1 + 0x18) = (short)(iVar2 >> 2);
    iVar2 = *(int *)(uVar5 + 0x18) - *(int *)(uVar5 + 0xc);
    if (iVar2 < 0) {
      iVar2 = iVar2 + 7;
    }
    *(short *)(iVar1 + 0x20) = (short)(iVar2 >> 3);
    FUN_8001d544(param_1,iVar1);
    uVar5 = *param_1;
    *param_1 = uVar5 | 0x20000;
    if ((uVar5 & 4) == 0) {
      FUN_800207c4(iVar1);
    }
    uVar3 = FUN_8004410c();
    FUN_8004483c(uVar3,uRam000005f8,0x2d,param_1 + 9);
  }
  else {
    uVar5 = param_1[0xe];
    if (uVar5 != 0) {
      do {
        if ((*(short *)(uVar5 + 10) == 0x26) && (*(int *)(uVar5 + 0x58) == DAT_800737dc)) break;
        uVar5 = *(uint *)(uVar5 + 0x34);
      } while (uVar5 != 0);
      if (uVar5 != 0) {
        FUN_8001ac08(uVar5);
      }
    }
  }
  return;
}

