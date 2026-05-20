// addr: 0x80018f7c  name: FUN_80018f7c

void FUN_80018f7c(int param_1,uint *param_2)

{
  undefined4 *puVar1;
  uint *puVar2;
  uint uVar3;
  
  puVar1 = *(undefined4 **)(param_1 + iRam00000004 * 4);
  uVar3 = *param_2;
  puVar2 = (uint *)*puVar1;
  *param_2 = (uint)(puVar1 + 1) & 0xffffff;
  *puVar2 = (uint)*(byte *)((int)puVar2 + 3) << 0x18 | uVar3;
  return;
}

