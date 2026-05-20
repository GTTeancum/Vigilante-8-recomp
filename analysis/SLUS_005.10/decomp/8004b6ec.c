// addr: 0x8004b6ec  name: TYPE_OBJ_178

undefined4 TYPE_OBJ_178(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  int unaff_s0;
  int unaff_s1;
  byte in_stack_00000818;
  byte in_stack_00000819;
  
  while( true ) {
    iVar1 = CdReady(param_1,&stack0x00000818);
    unaff_s0 = unaff_s0 + 1;
    if ((iVar1 == unaff_s1) || (9 < unaff_s0)) break;
    CdControl('\x1b',&stack0x00000010,(u_char *)0x0);
    param_1 = 0;
  }
  if (iVar1 == 1) {
    CdControl('\t',(u_char *)0x0,(u_char *)0x0);
    CdGetSector(&stack0x00000018,0x200);
    iVar1 = strncmp(&stack0x00000019,"CD001",5);
    uVar2 = 1;
    if (iVar1 == 0) {
      uVar2 = 2;
    }
  }
  else {
    uVar2 = 0x10;
    if ((in_stack_00000818 & 0x10) == 0) {
      if (((in_stack_00000818 & 1) != 0) && ((in_stack_00000819 & 0x40) != 0)) {
        printf("Command Error: ");
        uVar2 = TYPE_OBJ_250();
        return uVar2;
      }
      uVar2 = TYPE_OBJ_250();
      return uVar2;
    }
  }
  return uVar2;
}

