// addr: 0x8003ff28  name: FUN_8003ff28

int FUN_8003ff28(uint *param_1)

{
  ushort uVar1;
  undefined4 uVar2;
  MATRIX *m0;
  int iVar3;
  
  uVar2 = FUN_8001d5a0();
  m0 = (MATRIX *)FUN_8001d624(uVar2);
  FUN_800203fc(param_1);
  CompMatrixLV(m0,(MATRIX *)(param_1 + 4),(MATRIX *)(param_1 + 4));
  FUN_8001d564(param_1);
  *(undefined1 *)(param_1 + 1) = 1;
  param_1[0x19] = (uint)FUN_8003eab0;
  *param_1 = *param_1 & 0xffffbfff | 0x80;
  iVar3 = FUN_80045134(param_1,0x94);
  if (*(int *)(iVar3 + 0x38) != 0) {
    *(int *)(*(int *)(iVar3 + 0x38) + 0x3c) = iVar3;
  }
  *(int *)(iVar3 + 0x88) = (*(int *)(iVar3 + 0x48) << 9) >> 0xc;
  *(int *)(iVar3 + 0x8c) = (*(int *)(iVar3 + 0x4c) << 9) >> 0xc;
  *(int *)(iVar3 + 0x90) = (*(int *)(iVar3 + 0x50) << 9) >> 0xc;
  uVar1 = FUN_80017160();
  *(ushort *)(iVar3 + 0x80) = uVar1 & 0xff;
  uVar1 = FUN_80017160();
  *(ushort *)(iVar3 + 0x82) = uVar1 & 0xff;
  uVar1 = FUN_80017160();
  *(ushort *)(iVar3 + 0x84) = uVar1 & 0xff;
  *(undefined4 *)(iVar3 + 0x48) = *(undefined4 *)(iVar3 + 0x24);
  *(undefined4 *)(iVar3 + 0x4c) = *(undefined4 *)(iVar3 + 0x28);
  *(undefined4 *)(iVar3 + 0x50) = *(undefined4 *)(iVar3 + 0x2c);
  *(undefined1 *)(iVar3 + 0x87) = 2;
  FUN_80043358(m0,iVar3 + 0x88,iVar3 + 0x88);
  FUN_8002036c(iVar3);
  return iVar3;
}

