// addr: 0x80024520  name: FUN_80024520

void FUN_80024520(short *param_1,undefined4 param_2,int param_3,int param_4,int param_5,int param_6)

{
  ushort uVar1;
  undefined2 uVar2;
  
  param_6 = param_6 >> 1;
  uVar2 = (undefined2)param_2;
  if ((int)param_1[1] < param_5 + param_6) {
    if ((int)*param_1 < param_4 + param_6) {
      uVar1 = *(ushort *)(param_3 + 2);
      if ((uVar1 == 0) || ((uVar1 & 0x8000) != 0)) {
        *(undefined2 *)(param_3 + 2) = uVar2;
      }
      else {
        FUN_80024520(param_1,param_2,param_3 + (uint)uVar1 * 10,param_4,param_5,param_6);
      }
    }
    if (param_4 + param_6 < (int)*param_1 + (int)param_1[2]) {
      uVar1 = *(ushort *)(param_3 + 4);
      if ((uVar1 == 0) || ((uVar1 & 0x8000) != 0)) {
        *(undefined2 *)(param_3 + 4) = uVar2;
      }
      else {
        FUN_80024520(param_1,param_2,param_3 + (uint)uVar1 * 10,param_4 + param_6,param_5,param_6);
      }
    }
  }
  if (param_5 + param_6 < (int)param_1[1] + (int)param_1[3]) {
    if ((int)*param_1 < param_4 + param_6) {
      uVar1 = *(ushort *)(param_3 + 6);
      if ((uVar1 == 0) || ((uVar1 & 0x8000) != 0)) {
        *(undefined2 *)(param_3 + 6) = uVar2;
      }
      else {
        FUN_80024520(param_1,param_2,param_3 + (uint)uVar1 * 10,param_4,param_5 + param_6,param_6);
      }
    }
    if (param_4 + param_6 < (int)*param_1 + (int)param_1[2]) {
      uVar1 = *(ushort *)(param_3 + 8);
      if ((uVar1 == 0) || ((uVar1 & 0x8000) != 0)) {
        *(undefined2 *)(param_3 + 8) = uVar2;
      }
      else {
        FUN_80024520(param_1,param_2,param_3 + (uint)uVar1 * 10,param_4 + param_6,param_5 + param_6,
                     param_6);
      }
    }
  }
  return;
}

