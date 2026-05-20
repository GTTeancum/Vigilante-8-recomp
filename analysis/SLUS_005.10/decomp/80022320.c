// addr: 0x80022320  name: FUN_80022320

undefined4 FUN_80022320(uint *param_1,uint param_2)

{
  int iVar1;
  
  if ((*param_1 & 0x8000) == 0) {
    if ((ushort)param_1[3] < param_2) {
      *(undefined2 *)(param_1 + 3) = *(undefined2 *)((int)param_1 + 0xe);
      iVar1 = FUN_8003fc50();
      if (iVar1 != 0) {
        (*pcRam00000730)(param_1,0x11,0);
        return 1;
      }
    }
    else {
      *(ushort *)(param_1 + 3) = (ushort)param_1[3] - (short)param_2;
    }
  }
  return 0;
}

