# addr: 0x80016c88  name: FUN_80016c88
80016c88:  addiu sp,sp,-0x18
80016c8c:  move v0,a0
80016c90:  sw ra,0x10(sp)
80016c94:  lh a0,0x4(v0)
80016c98:  lh a1,0x10(v0)
80016c9c:  jal 0x8004ecd4
80016ca0:  _nop
80016ca4:  lw ra,0x10(sp)
80016ca8:  sll v0,v0,0x10
80016cac:  sra v0,v0,0x10
80016cb0:  jr ra
80016cb4:  _addiu sp,sp,0x18
