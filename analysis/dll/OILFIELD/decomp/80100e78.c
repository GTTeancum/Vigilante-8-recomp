// addr: 0x80100e78  name: FUN_80100e78

void FUN_80100e78(int param_1,uint param_2,undefined4 param_3)

{
  undefined2 uVar1;
  int iVar2;
  uint *puVar3;
  
  if (param_2 == 3) {
LAB_80100ecc:
    FUN_8002239c/*0x8002239c*/(param_1,param_3);
LAB_80100ee0:
    iVar2 = FUN_80022320/*0x80022320*/(param_1,param_3);
    param_3 = 0x98;
    if (iVar2 != 0) goto LAB_80100f18;
  }
  else if (((3 < param_2) || (param_2 != 1)) && (param_2 != 6)) {
    if (param_2 != 8) goto LAB_80100ecc;
    goto LAB_80100ee0;
  }
  *(uint *)(param_1 + 0x80) = (uint)*(ushort *)(*(int *)(param_1 + 0x38) + 10);
LAB_80100f18:
  puVar3 = (uint *)FUN_8001ac44/*0x8001ac44*/(*(undefined4 *)(param_1 + 0x58),*(undefined2 *)(param_1 + 0x80),
                                   param_3,8);
  FUN_8001d68c/*0x8001d68c*/(puVar3 + 4,param_1,
                  **(int **)(param_1 + 0x58) + *(int *)(param_1 + 0x80) * 0x1c + 0x1c);
  FUN_8001dc1c/*0x8001dc1c*/(puVar3);
  iVar2 = puVar3[0x15] * 0xc00;
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0xfff;
  }
  puVar3[0x15] = iVar2 >> 0xc;
  *(undefined2 *)((int)puVar3 + 6) = 1000;
  uVar1 = *(undefined2 *)(param_1 + 0xe);
  puVar3[0x19] = (uint)FUN_80100a30;
  puVar3[0x21] = 0xfffff415;
  iVar2 = puVar3[0x15] * 0x3243;
  *(undefined2 *)(puVar3 + 3) = uVar1;
  *puVar3 = *puVar3 | 0x88;
  if (iVar2 < 0) {
    iVar2 = iVar2 + 0xfff;
  }
  *(short *)(puVar3 + 0x25) = (short)(0x1000000 / (iVar2 >> 0xc));
  FUN_8003e76c/*0x8003e76c*/(puVar3);
  FUN_800202f4/*0x800202f4*/(puVar3);
  *(undefined1 **)(param_1 + 100) = &SUB_800223dc;
  return;
}

