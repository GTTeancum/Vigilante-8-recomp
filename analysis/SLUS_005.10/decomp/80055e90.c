// addr: 0x80055e90  name: PADPORTD_OBJ_2BC

void PADPORTD_OBJ_2BC(int param_1)

{
  undefined1 uVar1;
  
  uVar1 = *(undefined1 *)(param_1 + 0x36);
  *(undefined1 *)(param_1 + 0x36) = 0;
  *(undefined1 *)(param_1 + 0x37) = uVar1;
  return;
}

