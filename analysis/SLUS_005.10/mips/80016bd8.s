# addr: 0x80016bd8  name: FUN_80016bd8
80016bd8:  addiu sp,sp,-0x28
80016bdc:  sw ra,0x20(sp)
80016be0:  lw v0,0x0(a2)
80016be4:  lw v1,0x0(a1)
80016be8:  nop
80016bec:  subu v0,v0,v1
80016bf0:  sw v0,0x10(sp)
80016bf4:  lw v0,0x4(a2)
80016bf8:  lw v1,0x4(a1)
80016bfc:  move a3,a0
80016c00:  addiu a0,sp,0x10
80016c04:  subu v0,v0,v1
80016c08:  sw v0,0x4(a0)
80016c0c:  lw v0,0x8(a2)
80016c10:  lw v1,0x8(a1)
80016c14:  move a1,a3
80016c18:  subu v0,v0,v1
80016c1c:  jal 0x80016b08
80016c20:  _sw v0,0x8(a0)
80016c24:  lw ra,0x20(sp)
80016c28:  nop
80016c2c:  jr ra
80016c30:  _addiu sp,sp,0x28
