// addr: 0x80048490  name: INTR_OBJ_43C

int INTR_OBJ_43C(uint param_1,int param_2)

{
  ushort uVar1;
  int *piVar2;
  word wVar3;
  int iVar4;
  
  wVar3 = I_MASK;
  piVar2 = &DAT_8005eef0 + param_1;
  iVar4 = *piVar2;
  if ((param_2 != iVar4) && (DAT_8005eeec != 0)) {
    I_MASK = 0;
    if (param_2 != 0) {
      *piVar2 = param_2;
      DAT_8005ef1c = DAT_8005ef1c | (ushort)(1 << (param_1 & 0x1f));
      iVar4 = INTR_OBJ_4EC();
      return iVar4;
    }
    uVar1 = ~(ushort)(1 << (param_1 & 0x1f));
    *piVar2 = 0;
    wVar3 = wVar3 & uVar1;
    DAT_8005ef1c = DAT_8005ef1c & uVar1;
    if (param_1 == 0) {
      ChangeClearPAD(1);
      ChangeClearRCnt(3,1);
    }
    if (param_1 == 4) {
      ChangeClearRCnt(0,1);
    }
    if (param_1 == 5) {
      ChangeClearRCnt(1,1);
    }
    if (param_1 == 6) {
      ChangeClearRCnt(2,1);
    }
  }
  I_MASK = wVar3;
  return iVar4;
}

