# addr: 0x80016cb8  name: FUN_80016cb8
80016cb8:  addiu sp,sp,-0x18
80016cbc:  move v0,a0
80016cc0:  sw ra,0x10(sp)
80016cc4:  lh a0,0x6(v0)
80016cc8:  lh a1,0x0(v0)
80016ccc:  jal 0x8004ecd4
80016cd0:  _nop
80016cd4:  lw ra,0x10(sp)
80016cd8:  subu v0,zero,v0
80016cdc:  sll v0,v0,0x10
80016ce0:  sra v0,v0,0x10
80016ce4:  jr ra
80016ce8:  _addiu sp,sp,0x18
