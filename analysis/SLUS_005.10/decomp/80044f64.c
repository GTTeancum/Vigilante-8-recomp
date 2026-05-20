// addr: 0x80044f64  name: FUN_80044f64

/* WARNING: Instruction at (ram,0x80044f68) overlaps instruction at (ram,0x80044f64)
    */

void FUN_80044f64(undefined4 *param_1,int param_2)

{
  uint uVar1;
  uint *puVar2;
  undefined1 *puVar3;
  
  while( true ) {
    if (param_2 == 0) {
      return;
    }
    if (((uint)param_1 & 3) == 0) break;
    *(undefined1 *)param_1 = 0;
    param_2 = param_2 + -1;
    param_1 = (undefined4 *)((int)param_1 + 1);
  }
  while (-1 < param_2 + -4) {
    *param_1 = 0;
    param_1 = param_1 + 1;
    param_2 = param_2 + -4;
  }
  puVar3 = (undefined1 *)((int)param_1 + param_2 + -1);
  if (param_2 + -1 < 0) {
    return;
  }
  uVar1 = (uint)puVar3 & 3;
  puVar2 = (uint *)(puVar3 + -uVar1);
  *puVar2 = *puVar2 & -1 << (uVar1 + 1) * 8 | 0U >> (3 - uVar1) * 8;
  return;
}

