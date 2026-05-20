// addr: 0x8004b7e4  name: data_ready_callback

void data_ready_callback(void)

{
  uint uVar1;
  uint uVar2;
  undefined2 *puVar3;
  
  puVar3 = (undefined2 *)(DAT_800a32c8 + DAT_800a32b0 * 0x20);
  *puVar3 = 2;
  uVar1 = (int)puVar3 + 0x1fU & 3;
  uVar2 = (uint)(puVar3 + 0xe) & 3;
  DAT_800a3278 = (*(int *)(((int)puVar3 + 0x1fU) - uVar1) << (3 - uVar1) * 8 |
                 0xffffffffU >> (uVar1 + 1) * 8 & 2) & -1 << (4 - uVar2) * 8 |
                 *(uint *)((int)(puVar3 + 0xe) - uVar2) >> uVar2 * 8;
  DAT_800a327c = *(undefined4 *)(puVar3 + 4);
  DAT_800a32b0 = DAT_800a32ac;
  if (DAT_800a32d0 != (code *)0x0) {
    (*DAT_800a32d0)();
  }
  DAT_800a32a4 = 0;
  return;
}

