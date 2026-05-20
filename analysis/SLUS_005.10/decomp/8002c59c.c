// addr: 0x8002c59c  name: FUN_8002c59c

void FUN_8002c59c(uint *param_1,uint param_2)

{
  char cVar1;
  short sVar2;
  int iVar3;
  undefined4 uVar4;
  undefined2 uVar5;
  
  if ((param_2 & 0xffff) == 0) {
    FUN_800441c8((int)*(char *)((int)param_1 + 5));
  }
  else {
    *(undefined2 *)((int)param_1 + 0xa6) = 0;
    sVar2 = (short)param_1[0x2c] + -1;
    *(short *)(param_1 + 0x2c) = sVar2;
    if (sVar2 == -1) {
      iVar3 = (int)*(char *)((int)param_1 + 5);
      *param_1 = *param_1 & 0xf7ffffff;
      if (iVar3 == 0) {
        cVar1 = FUN_8004410c();
        *(char *)((int)param_1 + 5) = cVar1;
        iVar3 = (int)cVar1;
      }
      FUN_800443c8(iVar3,*(undefined4 *)((&DAT_800737a0)[0xf - *(short *)((int)param_1 + 6)] + 8),0,
                   0);
      uVar4 = FUN_8004410c();
      FUN_800447e8(uVar4,uRam000005f8,0x1f,param_1 + 9);
    }
    else if ((param_2 & 0xffff0000) != 0) {
      iVar3 = FUN_80017160();
      uVar5 = 0x13;
      if (iVar3 * 5 >> 0xf != 0) {
        uVar5 = 300;
      }
      *(undefined2 *)(param_1 + 0x2c) = uVar5;
      iVar3 = (int)*(char *)((int)param_1 + 5);
      if (iVar3 == 0) {
        cVar1 = FUN_8004410c();
        *(char *)((int)param_1 + 5) = cVar1;
        iVar3 = (int)cVar1;
      }
      FUN_800443c8(iVar3,uRam000005f8,0x21,0);
    }
  }
  return;
}

