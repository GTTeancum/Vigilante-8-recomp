// addr: 0x8001f3ac  name: FUN_8001f3ac

int FUN_8001f3ac(int param_1,MATRIX *param_2,undefined4 param_3,undefined4 param_4,
                undefined4 param_5)

{
  int iVar1;
  uint *puVar2;
  MATRIX MStack_40;
  
  puVar2 = *(uint **)(param_1 + 0x38);
  do {
    if (puVar2 == (uint *)0x0) {
      return 0;
    }
    if (puVar2[0x17] == 0) {
      if ((*puVar2 & 0x800) != 0) {
        CompMatrixLV(param_2,(MATRIX *)(puVar2 + 4),&MStack_40);
        goto LAB_8001f4c8;
      }
    }
    else {
      CompMatrixLV(param_2,(MATRIX *)(puVar2 + 4),&MStack_40);
      if (((0 < MStack_40.m[1][1]) ||
          (0x800 < (int)MStack_40.m[0][1] * (int)*(short *)(param_1 + 0x12) +
                   (int)MStack_40.m[1][1] * (int)*(short *)(param_1 + 0x18) +
                   (int)MStack_40.m[2][1] * (int)*(short *)(param_1 + 0x1e))) &&
         (iVar1 = FUN_8001ef74(puVar2[0x17],&MStack_40,param_3,param_4,param_5), iVar1 != 0)) {
        return iVar1;
      }
      if ((*puVar2 & 0x800) != 0) {
LAB_8001f4c8:
        iVar1 = FUN_8001f3ac(puVar2,&MStack_40,param_3,param_4,param_5);
        if (iVar1 != 0) {
          return iVar1;
        }
      }
    }
    puVar2 = (uint *)puVar2[0xd];
  } while( true );
}

