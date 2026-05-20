// addr: 0x80017e3c  name: FUN_80017e3c

void FUN_80017e3c(undefined4 param_1,undefined4 param_2,undefined2 param_3,undefined2 param_4,
                 undefined2 param_5,undefined2 param_6)

{
  undefined2 *puVar1;
  
  puVar1 = (undefined2 *)FUN_800116f4(0x18);
  *(undefined4 *)(puVar1 + 6) = param_1;
  *(undefined4 *)(puVar1 + 4) = param_2;
  *puVar1 = param_3;
  puVar1[1] = param_4;
  puVar1[2] = param_5;
  puVar1[3] = param_6;
  iRam000006c8 = iRam000006c8 + 1;
  return;
}

