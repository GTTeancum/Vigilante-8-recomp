# addr: 0x80043bb4  name: FUN_80043bb4
80043bb4:  addiu sp,sp,-0x20
80043bb8:  li a0,0x1
80043bbc:  clear a1
80043bc0:  sw ra,0x18(sp)
80043bc4:  jal 0x80049240
80043bc8:  _addiu a2,sp,0x10
80043bcc:  lbu v0,0x10(sp)
80043bd0:  lw ra,0x18(sp)
80043bd4:  srl v0,v0,0x4
80043bd8:  andi v0,v0,0x1
80043bdc:  jr ra
80043be0:  _addiu sp,sp,0x20
