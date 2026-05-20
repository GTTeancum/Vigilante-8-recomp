// addr: 0x80018efc  name: FUN_80018efc

undefined4 * FUN_80018efc(undefined4 param_1)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  
  puVar1 = (undefined4 *)FUN_800116f4(8);
  *puVar1 = param_1;
  uVar2 = FUN_80018e28();
  puVar1[1] = uVar2;
  return puVar1;
}

