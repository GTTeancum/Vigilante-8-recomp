// addr: 0x8002c4bc  name: FUN_8002c4bc

void FUN_8002c4bc(uint *param_1)

{
  char cVar1;
  int iVar2;
  int iVar3;
  
  if (((*param_1 & 0x8000000) == 0) && ((char)param_1[0x34] != '\f')) {
    *param_1 = *param_1 | 0x8000000;
    iVar2 = FUN_80017160();
    iVar3 = (int)*(char *)((int)param_1 + 5);
    *(short *)(param_1 + 0x2c) = (short)(iVar2 * 0xb4 >> 0xf) + 0xb4;
    *(undefined2 *)((int)param_1 + 0xa6) = 0xff88;
    if (iVar3 == 0) {
      iVar3 = FUN_8004410c();
    }
    FUN_8004483c(iVar3,uRam000005f8,0x20,param_1 + 9);
    if (*(short *)((int)param_1 + 6) < 0) {
      *(undefined1 *)((int)param_1 + 5) = 0;
    }
    else {
      cVar1 = FUN_8004410c();
      *(char *)((int)param_1 + 5) = cVar1;
      FUN_800443c8((int)cVar1,uRam000005f8,0x21,0);
    }
  }
  return;
}

