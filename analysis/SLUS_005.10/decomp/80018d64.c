// addr: 0x80018d64  name: FUN_80018d64

undefined4 * FUN_80018d64(undefined4 *param_1,int param_2)

{
  int iVar1;
  uint *p1;
  uint *p0;
  int iVar2;
  
  p0 = param_1 + 1;
  iVar2 = 0;
  if (0 < param_2 + -1) {
    do {
      p1 = p0 + *(byte *)((int)p0 + 3) + 1;
      iVar1 = MargePrim(p0,p1);
      if (iVar1 < 0) {
        *p0 = *p0 & 0xff000000 | (uint)p1 & 0xffffff;
        p0 = p1;
      }
      iVar2 = iVar2 + 1;
    } while (iVar2 < param_2 + -1);
  }
  *param_1 = p0;
  return param_1;
}

