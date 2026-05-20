// addr: 0x8001d748  name: FUN_8001d748

int FUN_8001d748(int param_1,undefined4 *param_2,SVECTOR *param_3,undefined4 *param_4)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  
  iVar1 = FUN_80025400(*param_2,param_2[2]);
  if ((*(int *)(param_1 + 0x74) == 0) ||
     ((iVar2 = FUN_8001f51c(*(int *)(param_1 + 0x74),iVar1,param_2,param_3), iVar2 == 0 &&
      ((*(int *)(param_1 + 0x78) == 0 ||
       (iVar2 = FUN_8001f51c(*(int *)(param_1 + 0x78),iVar1,param_2,param_3), iVar2 == 0)))))) {
    if (param_3 != (SVECTOR *)0x0) {
      FUN_80025648(*param_2,param_2[2],param_3);
      VectorNormalSS(param_3,param_3);
    }
    if (param_4 != (undefined4 *)0x0) {
      uVar3 = FUN_800255f4(*param_2,param_2[2]);
      *param_4 = uVar3;
    }
  }
  else {
    iVar1 = iVar2;
    if (param_4 != (undefined4 *)0x0) {
      *param_4 = 0;
    }
  }
  return iVar1;
}

