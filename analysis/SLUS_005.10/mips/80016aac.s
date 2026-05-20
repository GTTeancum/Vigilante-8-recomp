# addr: 0x80016aac  name: FUN_80016aac
80016aac:  addiu sp,sp,-0x28
80016ab0:  move a2,a0
80016ab4:  sw ra,0x20(sp)
80016ab8:  lw v0,0x0(a2)
80016abc:  lw v1,0x0(a1)
80016ac0:  nop
80016ac4:  subu v0,v0,v1
80016ac8:  sw v0,0x10(sp)
80016acc:  lw v0,0x4(a2)
80016ad0:  lw v1,0x4(a1)
80016ad4:  addiu a0,sp,0x10
80016ad8:  subu v0,v0,v1
80016adc:  sw v0,0x4(a0)
80016ae0:  lw v0,0x8(a2)
80016ae4:  lw v1,0x8(a1)
80016ae8:  nop
80016aec:  subu v0,v0,v1
80016af0:  jal 0x80016a20
80016af4:  _sw v0,0x8(a0)
80016af8:  lw ra,0x20(sp)
80016afc:  nop
80016b00:  jr ra
80016b04:  _addiu sp,sp,0x28
