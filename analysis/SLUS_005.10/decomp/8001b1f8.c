// addr: 0x8001b1f8  name: FUN_8001b1f8

ushort * FUN_8001b1f8(int param_1)

{
  ushort uVar1;
  ushort *puVar2;
  
  uVar1 = *(ushort *)(**(int **)(param_1 + 0x58) + (uint)*(ushort *)(param_1 + 10) * 0x1c + 0x36);
  while( true ) {
    if (uVar1 == 0xffff) {
      return (ushort *)0x0;
    }
    puVar2 = (ushort *)(**(int **)(param_1 + 0x58) + (uint)uVar1 * 0x1c + 0x1c);
    if (*puVar2 >> 0xc == 0xb) break;
    uVar1 = puVar2[0xc];
  }
  return puVar2;
}

