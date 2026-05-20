// addr: 0x8002b7bc  name: FUN_8002b7bc

void FUN_8002b7bc(int param_1,undefined4 param_2,void *param_3)

{
  short sVar1;
  uint *puVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  
  iVar4 = *(int *)(param_1 + 0xe4);
  if (iVar4 != 0) {
    puVar2 = *(uint **)(param_1 + (*(byte *)(param_1 + 0xb3) + 9) * 4 + 0xec);
    if (*(short *)(iVar4 + 0x120) == 0) {
      uVar3 = 0;
      iVar5 = iVar4 + 0x24;
      if (puVar2 != (uint *)0x0) {
        uVar3 = (uint)((*puVar2 & 0x4000) != 0);
      }
      sVar1 = *(short *)(param_1 + 0xbc);
      uVar6 = *(undefined4 *)(iVar4 + 0x54);
      uVar7 = (&DAT_80065b70)[uVar3];
    }
    else {
      sVar1 = *(short *)(param_1 + 0xbc);
      uVar6 = *(undefined4 *)(iVar4 + 0x54);
      iVar5 = iVar4 + 0x48;
      uVar7 = uRam00000874;
    }
    FUN_8002b610(iVar5,uVar6,uVar7,param_2,(int)sVar1,param_3);
  }
  if (*(short *)(param_1 + 0x120) != 0) {
    FUN_8002b610(param_1 + 0x48,*(undefined4 *)(param_1 + 0x54),uRam00000878,param_2,0x100,param_3);
  }
  AddPrim(param_3,&UNK_80065b60 + iRam00000004 * 8);
  return;
}

