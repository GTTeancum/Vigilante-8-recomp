# addr: 0x80101ca8  name: FUN_80101ca8
80101ca8:  addiu sp,sp,-0x18
80101cac:  sw ra,0x10(sp)
80101cb0:  li v0,0x1
80101cb4:  beq a1,v0,0x80101ce4
80101cb8:  _move v1,a0
80101cbc:  li v0,0x7
80101cc0:  bne a1,v0,0x80101cf0
80101cc4:  _clear v0
80101cc8:  lui v0,0x8006
80101ccc:  lw v0,0x59fc(v0)
80101cd0:  li a0,0x80
80101cd4:  jal 0x8001d470
80101cd8:  _sw v1,0x58(v0)
80101cdc:  slti a0,zero,0x73c
80101ce0:  nop
80101ce4:  jal 0x80045088
80101ce8:  _move a0,v1
80101cec:  li v0,-0x1
80101cf0:  lw ra,0x10(sp)
80101cf4:  nop
80101cf8:  jr ra
80101cfc:  _addiu sp,sp,0x18
