// addr: 0x8001add0  name: FUN_8001add0

int FUN_8001add0(int param_1,int *param_2,ushort param_3,uint param_4)

{
  undefined4 uVar1;
  int iVar2;
  ushort *puVar3;
  
  puVar3 = (ushort *)(*param_2 + (uint)param_3 * 0x1c + 0x1c);
  if ((short)*puVar3 < 0) {
    *(undefined4 *)(param_1 + 0x30) = 0;
  }
  else {
    uVar1 = FUN_8001b49c(param_2,*puVar3 & 0x7ff);
    *(undefined4 *)(param_1 + 0x30) = uVar1;
  }
  if ((short)puVar3[1] < 0) {
    *(undefined4 *)(param_1 + 0x5c) = 0;
  }
  else {
    *(undefined4 *)(param_1 + 0x5c) =
         *(undefined4 *)((short)puVar3[1] * 4 + *(int *)(*param_2 + 0xc));
  }
  *(int **)(param_1 + 0x58) = param_2;
  *(ushort *)(param_1 + 10) = param_3;
  if ((param_4 & 8) == 0) {
    *(undefined4 *)(param_1 + 0x60) = 0;
  }
  else {
    uVar1 = FUN_8001ab98(param_2,param_3);
    *(undefined4 *)(param_1 + 0x60) = uVar1;
  }
  *(undefined2 *)(param_1 + 0x46) = uRam000006cc;
  if (((param_4 & 1) == 0) || (puVar3[0xc] == 0xffff)) {
    *(undefined4 *)(param_1 + 0x34) = 0;
  }
  else {
    iVar2 = FUN_8001ac44(param_2,puVar3[0xc],0x80,param_4);
    *(int *)(param_1 + 0x34) = iVar2;
    *(int *)(iVar2 + 0x3c) = param_1;
  }
  if (((param_4 & 2) == 0) && (puVar3[0xd] != 0xffff)) {
    iVar2 = FUN_8001ac44(param_2,puVar3[0xd],0x80,param_4 | 1);
    *(int *)(param_1 + 0x38) = iVar2;
    *(int *)(iVar2 + 0x3c) = param_1;
  }
  else {
    *(undefined4 *)(param_1 + 0x38) = 0;
  }
  *(undefined4 *)(param_1 + 0x3c) = 0;
  return param_1;
}

