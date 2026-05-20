// addr: 0x8003cee0  name: FUN_8003cee0

int FUN_8003cee0(undefined4 param_1,undefined4 *param_2,undefined4 *param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  
  iVar1 = FUN_8003ce24();
  iVar1 = FUN_8001ac44(uRam000008b4,*(undefined2 *)(&DAT_8005ec84 + iVar1 * 2),0x94,0);
  *(undefined1 **)(iVar1 + 100) = &LAB_8003cd0c;
  uVar2 = param_2[1];
  uVar3 = param_2[2];
  *(undefined4 *)(iVar1 + 0x48) = *param_2;
  *(undefined4 *)(iVar1 + 0x4c) = uVar2;
  *(undefined4 *)(iVar1 + 0x50) = uVar3;
  uVar2 = param_3[1];
  uVar3 = param_3[2];
  *(undefined4 *)(iVar1 + 0x88) = *param_3;
  *(undefined4 *)(iVar1 + 0x8c) = uVar2;
  *(undefined4 *)(iVar1 + 0x90) = uVar3;
  *(undefined1 *)(iVar1 + 0x87) = 2;
  FUN_8002036c(iVar1);
  return iVar1;
}

