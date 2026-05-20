// addr: 0x8001ecc4  name: FUN_8001ecc4

int FUN_8001ecc4(int param_1,int param_2,MATRIX *param_3)

{
  int iVar1;
  uint *puVar2;
  MATRIX MStack_30;
  
  puVar2 = *(uint **)(param_2 + 0x38);
  do {
    if (puVar2 == (uint *)0x0) {
      return 0;
    }
    if ((puVar2[0x17] == 0) || ((*puVar2 & 0x20) != 0)) {
      if ((*puVar2 & 0x800) != 0) {
        CompMatrixLV(param_3,(MATRIX *)(puVar2 + 4),&MStack_30);
        goto LAB_8001ed78;
      }
    }
    else {
      CompMatrixLV(param_3,(MATRIX *)(puVar2 + 4),&MStack_30);
      iVar1 = FUN_8001e9a0(param_1,puVar2,param_1 + 0x10,&MStack_30);
      if (iVar1 != 0) {
        return iVar1;
      }
      if ((*puVar2 & 0x800) != 0) {
LAB_8001ed78:
        iVar1 = FUN_8001ecc4(param_1,puVar2,&MStack_30);
        if (iVar1 != 0) {
          return iVar1;
        }
      }
    }
    puVar2 = (uint *)puVar2[0xd];
  } while( true );
}

