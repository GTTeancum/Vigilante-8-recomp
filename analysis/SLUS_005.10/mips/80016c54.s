# addr: 0x80016c54  name: FUN_80016c54
80016c54:  addiu sp,sp,-0x18
80016c58:  move v0,a0
80016c5c:  sw ra,0x10(sp)
80016c60:  lh a0,0xa(v0)
80016c64:  lh a1,0x10(v0)
80016c68:  jal 0x8004ecd4
80016c6c:  _nop
80016c70:  lw ra,0x10(sp)
80016c74:  subu v0,zero,v0
80016c78:  sll v0,v0,0x10
80016c7c:  sra v0,v0,0x10
80016c80:  jr ra
80016c84:  _addiu sp,sp,0x18
