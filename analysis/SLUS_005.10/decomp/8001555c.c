// addr: 0x8001555c  name: FUN_8001555c

uint FUN_8001555c(uint param_1)

{
  uint uVar1;
  uint *puVar2;
  undefined1 auStack_808 [40];
  uint uStack_7e0;
  uint uStack_7dc;
  uint uStack_7d8;
  uint uStack_7d4;
  uint uStack_7d0;
  uint uStack_7cc;
  uint uStack_7c8;
  uint uStack_7c4;
  
  FUN_800154f4(auStack_808,0x10,1);
  uVar1 = param_1 + 3 & 3;
  puVar2 = (uint *)((param_1 + 3) - uVar1);
  *puVar2 = *puVar2 & -1 << (uVar1 + 1) * 8 | uStack_7e0 >> (3 - uVar1) * 8;
  uVar1 = param_1 & 3;
  *(uint *)(param_1 - uVar1) =
       *(uint *)(param_1 - uVar1) & 0xffffffffU >> (4 - uVar1) * 8 | uStack_7e0 << uVar1 * 8;
  uVar1 = param_1 + 7 & 3;
  puVar2 = (uint *)((param_1 + 7) - uVar1);
  *puVar2 = *puVar2 & -1 << (uVar1 + 1) * 8 | uStack_7dc >> (3 - uVar1) * 8;
  uVar1 = param_1 + 4 & 3;
  puVar2 = (uint *)((param_1 + 4) - uVar1);
  *puVar2 = *puVar2 & 0xffffffffU >> (4 - uVar1) * 8 | uStack_7dc << uVar1 * 8;
  uVar1 = param_1 + 0xb & 3;
  puVar2 = (uint *)((param_1 + 0xb) - uVar1);
  *puVar2 = *puVar2 & -1 << (uVar1 + 1) * 8 | uStack_7d8 >> (3 - uVar1) * 8;
  uVar1 = param_1 + 8 & 3;
  puVar2 = (uint *)((param_1 + 8) - uVar1);
  *puVar2 = *puVar2 & 0xffffffffU >> (4 - uVar1) * 8 | uStack_7d8 << uVar1 * 8;
  uVar1 = param_1 + 0xf & 3;
  puVar2 = (uint *)((param_1 + 0xf) - uVar1);
  *puVar2 = *puVar2 & -1 << (uVar1 + 1) * 8 | uStack_7d4 >> (3 - uVar1) * 8;
  uVar1 = param_1 + 0xc & 3;
  puVar2 = (uint *)((param_1 + 0xc) - uVar1);
  *puVar2 = *puVar2 & 0xffffffffU >> (4 - uVar1) * 8 | uStack_7d4 << uVar1 * 8;
  uVar1 = param_1 + 0x13 & 3;
  puVar2 = (uint *)((param_1 + 0x13) - uVar1);
  *puVar2 = *puVar2 & -1 << (uVar1 + 1) * 8 | uStack_7d0 >> (3 - uVar1) * 8;
  uVar1 = param_1 + 0x10 & 3;
  puVar2 = (uint *)((param_1 + 0x10) - uVar1);
  *puVar2 = *puVar2 & 0xffffffffU >> (4 - uVar1) * 8 | uStack_7d0 << uVar1 * 8;
  uVar1 = param_1 + 0x17 & 3;
  puVar2 = (uint *)((param_1 + 0x17) - uVar1);
  *puVar2 = *puVar2 & -1 << (uVar1 + 1) * 8 | uStack_7cc >> (3 - uVar1) * 8;
  uVar1 = param_1 + 0x14 & 3;
  puVar2 = (uint *)((param_1 + 0x14) - uVar1);
  *puVar2 = *puVar2 & 0xffffffffU >> (4 - uVar1) * 8 | uStack_7cc << uVar1 * 8;
  uVar1 = param_1 + 0x1b & 3;
  puVar2 = (uint *)((param_1 + 0x1b) - uVar1);
  *puVar2 = *puVar2 & -1 << (uVar1 + 1) * 8 | uStack_7c8 >> (3 - uVar1) * 8;
  uVar1 = param_1 + 0x18 & 3;
  puVar2 = (uint *)((param_1 + 0x18) - uVar1);
  *puVar2 = *puVar2 & 0xffffffffU >> (4 - uVar1) * 8 | uStack_7c8 << uVar1 * 8;
  uVar1 = param_1 + 0x1f & 3;
  puVar2 = (uint *)((param_1 + 0x1f) - uVar1);
  *puVar2 = *puVar2 & -1 << (uVar1 + 1) * 8 | uStack_7c4 >> (3 - uVar1) * 8;
  uVar1 = param_1 + 0x1c & 3;
  puVar2 = (uint *)((param_1 + 0x1c) - uVar1);
  *puVar2 = *puVar2 & 0xffffffffU >> (4 - uVar1) * 8 | uStack_7c4 << uVar1 * 8;
  return param_1;
}

