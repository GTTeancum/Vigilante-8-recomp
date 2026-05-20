// addr: 0x8001ac44  name: FUN_8001ac44

int FUN_8001ac44(int *param_1,ushort param_2,undefined4 param_3,uint param_4)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  short *psVar4;
  
  psVar4 = (short *)(*param_1 + (uint)param_2 * 0x1c + 0x1c);
  if ((*psVar4 < 0) && ((*psVar4 != -1 || ((param_4 & 4) != 0)))) {
    iVar1 = 0;
    if ((param_4 & 1) != 0) {
      if (psVar4[0xc] == -1) {
        iVar1 = 0;
      }
      else {
        iVar1 = FUN_8001ac44(param_1,psVar4[0xc],0x80,param_4);
      }
    }
  }
  else {
    iVar1 = FUN_8001aaa8(param_1,psVar4);
    *(ushort *)(iVar1 + 10) = param_2;
    *(short *)(iVar1 + 6) = psVar4[0xb];
    if ((param_4 & 8) == 0) {
      *(undefined4 *)(iVar1 + 0x60) = 0;
    }
    else {
      uVar2 = FUN_8001ab98(param_1,param_2);
      *(undefined4 *)(iVar1 + 0x60) = uVar2;
    }
    *(undefined2 *)(iVar1 + 0x46) = uRam000006cc;
    if (((param_4 & 1) == 0) || (psVar4[0xc] == -1)) {
      *(undefined4 *)(iVar1 + 0x34) = 0;
    }
    else {
      iVar3 = FUN_8001ac44(param_1,psVar4[0xc],0x80,param_4);
      *(int *)(iVar1 + 0x34) = iVar3;
      *(int *)(iVar3 + 0x3c) = iVar1;
    }
    if (((param_4 & 2) == 0) && (psVar4[0xd] != -1)) {
      iVar3 = FUN_8001ac44(param_1,psVar4[0xd],0x80,param_4 | 1);
      *(int *)(iVar1 + 0x38) = iVar3;
      *(int *)(iVar3 + 0x3c) = iVar1;
    }
    else {
      *(undefined4 *)(iVar1 + 0x38) = 0;
    }
    *(undefined4 *)(iVar1 + 0x3c) = 0;
  }
  return iVar1;
}

