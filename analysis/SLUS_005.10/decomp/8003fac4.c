// addr: 0x8003fac4  name: FUN_8003fac4

void FUN_8003fac4(uint *param_1,int *param_2,ushort param_3)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  undefined1 auStack_40 [32];
  
  iVar3 = *param_2 + (uint)param_3 * 0x1c + 0x1c;
  uVar1 = FUN_8001d68c(auStack_40,param_1,iVar3);
  uVar1 = FUN_8003f4a0(param_2,*(undefined2 *)(iVar3 + 0x1a),uVar1,1);
  if ((*(short *)(iVar3 + 0x16) == -0x5556) || (*(short *)(iVar3 + 0x16) == 0)) {
    FUN_8003f89c(param_1,param_2,(uint)param_3,uVar1);
  }
  else {
    iVar2 = FUN_8001d470(0x80);
    *(int **)(iVar2 + 0x58) = param_2;
    *(ushort *)(iVar2 + 10) = param_3;
    *(undefined1 **)(iVar2 + 100) = &LAB_8003fa78;
    *(uint **)(iVar2 + 0x74) = param_1;
    *(undefined4 *)(iVar2 + 0x38) = uVar1;
    *param_1 = *param_1 | 0x8000;
    FUN_80020890(iVar2,*(undefined2 *)(iVar3 + 0x16));
  }
  return;
}

