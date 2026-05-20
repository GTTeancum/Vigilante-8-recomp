// addr: 0x80022cd0  name: FUN_80022cd0

void FUN_80022cd0(int param_1,SVECTOR *param_2,undefined4 param_3)

{
  ApplyMatrix((MATRIX *)&DAT_8006f680,param_2,(VECTOR *)&DAT_8005e958);
  DAT_8005e958 = DAT_8005e958 << 6;
  DAT_8005e95c = DAT_8005e95c << 6;
  DAT_8005e960 = DAT_8005e960 << 6;
  FUN_8001be5c(*(undefined4 *)(param_1 + 0x30),&DAT_8005e944,param_3);
  return;
}

