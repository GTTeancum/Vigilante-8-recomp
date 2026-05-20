// addr: 0x8003fdcc  name: FUN_8003fdcc

undefined4 * FUN_8003fdcc(undefined4 *param_1,undefined2 param_2,int param_3)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  
  puVar1 = (undefined4 *)FUN_8001ac44(DAT_800737d8,param_2,0x80,8);
  *(undefined1 *)(puVar1 + 1) = 7;
  if (param_3 == 0) {
    uVar2 = 0x124;
  }
  else {
    uVar2 = 0x104;
  }
  *puVar1 = uVar2;
  puVar1[0x15] = *(undefined4 *)(puVar1[0x17] + 0x10);
  uVar2 = param_1[1];
  uVar3 = param_1[2];
  puVar1[0x12] = *param_1;
  puVar1[0x13] = uVar2;
  puVar1[0x14] = uVar3;
  puVar1[0x19] = &LAB_8003efc8;
  *(short *)(puVar1 + 3) = (short)(param_3 / 0xc);
  *(undefined1 **)(puVar1[0xe] + 100) = &LAB_8003e7b4;
  FUN_8002036c(puVar1);
  return puVar1;
}

