// addr: 0x80019c64  name: FUN_80019c64

void FUN_80019c64(void)

{
  undefined4 *puVar1;
  uint *puVar2;
  uint uVar3;
  uint *in_stack_00000010;
  
  puVar1 = (undefined4 *)FUN_800197f4();
  puVar2 = (uint *)*puVar1;
  uVar3 = *in_stack_00000010;
  *in_stack_00000010 = (uint)(puVar1 + 1) & 0xffffff;
  *puVar2 = (uint)*(byte *)((int)puVar2 + 3) << 0x18 | uVar3;
  FUN_800118b4(puVar1);
  return;
}

