# addr: 0x80018bd0  name: FUN_80018bd0
80018bd0:  addiu sp,sp,-0x18
80018bd4:  lui v1,0xe100
80018bd8:  ori v1,v1,0x400
80018bdc:  li v0,0x1
80018be0:  sw ra,0x10(sp)
80018be4:  sb v0,0x3(a0)
80018be8:  li v0,0x4
80018bec:  sb v0,0xb(a0)
80018bf0:  li v0,0x65
80018bf4:  addiu a1,a0,0x8
80018bf8:  sw v1,0x4(a0)
80018bfc:  jal 0x80052344
80018c00:  _sb v0,0xf(a0)
80018c04:  lw ra,0x10(sp)
80018c08:  nop
80018c0c:  jr ra
80018c10:  _addiu sp,sp,0x18
