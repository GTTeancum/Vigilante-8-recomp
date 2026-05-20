// addr: 0x8010183c  name: FUN_8010183c

undefined4 * FUN_8010183c(int param_1,int param_2)

{
  undefined2 uVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  int iVar4;
  
  puVar2 = (undefined4 *)FUN_8001ac44/*0x8001ac44*/(*(undefined4 *)(param_1 + 0x58),0x1b7,0x98,8);
  uVar3 = FUN_8001b038/*0x8001b038*/(param_1,0x8000);
  FUN_8001d68c/*0x8001d68c*/(puVar2 + 4,param_1,uVar3);
  puVar2[0x12] = puVar2[9];
  puVar2[0x13] = puVar2[10];
  puVar2[0x14] = puVar2[0xb];
  *puVar2 = 0x84;
  *(undefined1 *)(puVar2 + 1) = 7;
  uVar1 = *(undefined2 *)(param_1 + 6);
  *(undefined2 *)(puVar2 + 3) = 0;
  puVar2[0x19] = FUN_801010f4;
  *(undefined2 *)((int)puVar2 + 6) = uVar1;
  iVar4 = *(int *)(param_2 + 0xe4);
  if (*(int *)(param_2 + 0xe4) == 0) {
    iVar4 = param_2;
  }
  puVar2[0x21] = iVar4;
  FUN_8002036c/*0x8002036c*/(puVar2);
  return puVar2;
}

