// addr: 0x800523a0  name: FUN_800523a0

/* Possible BCMP.OBJ/BCMP_OBJ_1C
   Possible MEMCMP.OBJ/MEMCMP_OBJ_1C */

int FUN_800523a0(byte *param_1,byte *param_2,int param_3)

{
  do {
    if (*param_1 != *param_2) {
      return (uint)*param_1 - (uint)*param_2;
    }
    param_3 = param_3 + -1;
    param_2 = param_2 + 1;
    param_1 = param_1 + 1;
  } while (0 < param_3);
  return 0;
}

