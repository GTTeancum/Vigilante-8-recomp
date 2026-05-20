// addr: 0x8002ee94  name: FUN_8002ee94

void FUN_8002ee94(uint *param_1,uint param_2)

{
  char cVar1;
  undefined4 uVar2;
  uint uVar3;
  undefined4 uVar4;
  
  if ((param_2 & 0xffff) == 0) {
    cVar1 = *(char *)((int)param_1 + 0xb5) + -1;
    if (*(char *)((int)param_1 + 0xb5) < '\x01') {
      return;
    }
  }
  else {
    if (((param_2 & 0xffff0000) != 0) && ((*param_1 & 0x100000) != 0)) {
      if (('\0' < *(char *)((int)param_1 + 0xb5)) &&
         (((int)param_1[0x23] < 0x20c8 && ('\0' < *(char *)((int)param_1 + 0xb2))))) {
        FUN_80017594(param_1,&DAT_80065748,&DAT_80065754);
        *(undefined1 *)((int)param_1 + 0xb5) = 0xd9;
        uVar2 = FUN_8004410c();
        uVar3 = FUN_80017160();
        uVar4 = 0x1b;
        if ((uVar3 & 1) != 0) {
          uVar4 = 0x1c;
        }
        FUN_8004483c(uVar2,uRam000005f8,uVar4,param_1 + 9);
      }
      if ((int)param_1[0x23] < 0x8f0) {
        uVar2 = FUN_8004410c();
        uVar3 = FUN_80017160();
        uVar4 = 0x1b;
        if ((uVar3 & 1) != 0) {
          uVar4 = 0x1c;
        }
        FUN_8004483c(uVar2,uRam000005f8,uVar4,param_1 + 9);
      }
    }
    cVar1 = *(char *)((int)param_1 + 0xb5) + '\x01';
    if (-2 < *(char *)((int)param_1 + 0xb5)) {
      cVar1 = '\x0f';
    }
  }
  *(char *)((int)param_1 + 0xb5) = cVar1;
  return;
}

