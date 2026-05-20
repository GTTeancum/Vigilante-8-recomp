// addr: 0x80101b54  name: FUN_80101b54

void FUN_80101b54(int param_1,undefined1 *param_2,int *param_3)

{
  short sVar1;
  undefined1 *in_v0;
  int iVar2;
  int *piVar3;
  int iVar4;
  short sVar5;
  
  iVar4 = 0;
  sVar5 = 1;
  piVar3 = param_3;
  if (in_v0 != (undefined1 *)0x0) {
    *in_v0 = 0;
    iVar4 = FUN_80019138/*0x80019138*/(param_3,param_2);
    if (iVar4 < 0) {
      iVar4 = 0;
    }
    *in_v0 = 10;
    sVar5 = 2;
    piVar3 = (int *)0x1;
    param_2 = in_v0 + 1;
  }
  iVar2 = FUN_80019138/*0x80019138*/(piVar3,param_2);
  sVar1 = (short)iVar2;
  if (iVar2 < iVar4) {
    sVar1 = (short)iVar4;
  }
  *(short *)(param_1 + 4) = sVar1 + 0x10;
  *(ushort *)(param_1 + 6) = sVar5 * (ushort)*(byte *)(*param_3 + 6) + 0x10;
  return;
}

