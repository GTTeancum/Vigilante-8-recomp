# addr: 0x8004fb18  name: DrawPrim
8004fb18:  addiu sp,sp,-0x20
8004fb1c:  sw s0,0x10(sp)
8004fb20:  move s0,a0
8004fb24:  lui v0,0x8006
8004fb28:  lw v0,0x501c(v0)
8004fb2c:  sw ra,0x18(sp)
8004fb30:  sw s1,0x14(sp)
8004fb34:  lw v0,0x3c(v0)
8004fb38:  lbu s1,0x3(s0)
8004fb3c:  jalr v0
8004fb40:  _clear a0
8004fb44:  lui v0,0x8006
8004fb48:  lw v0,0x501c(v0)
8004fb4c:  addiu a0,s0,0x4
8004fb50:  lw v0,0x14(v0)
8004fb54:  nop
8004fb58:  jalr v0
8004fb5c:  _move a1,s1
8004fb60:  lw ra,0x18(sp)
8004fb64:  lw s1,0x14(sp)
8004fb68:  lw s0,0x10(sp)
8004fb6c:  jr ra
8004fb70:  _addiu sp,sp,0x20
