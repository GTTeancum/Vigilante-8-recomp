# addr: 0x8002ee94  name: FUN_8002ee94
8002ee94:  addiu sp,sp,-0x20
8002ee98:  sw s2,0x18(sp)
8002ee9c:  move s2,a0
8002eea0:  sw s0,0x10(sp)
8002eea4:  addiu s0,s2,0xa4
8002eea8:  andi v0,a1,0xffff
8002eeac:  sw ra,0x1c(sp)
8002eeb0:  beq v0,zero,0x8002efb4
8002eeb4:  _sw s1,0x14(sp)
8002eeb8:  lui v0,0xffff
8002eebc:  and v0,a1,v0
8002eec0:  beq v0,zero,0x8002ef98
8002eec4:  _lui v1,0x10
8002eec8:  lw v0,0x0(s2)
8002eecc:  nop
8002eed0:  and v0,v0,v1
8002eed4:  beq v0,zero,0x8002ef98
8002eed8:  _nop
8002eedc:  lb v0,0x11(s0)
8002eee0:  nop
8002eee4:  blez v0,0x8002ef54
8002eee8:  _nop
8002eeec:  lw v0,0x8c(s2)
8002eef0:  nop
8002eef4:  slti v0,v0,0x20c8
8002eef8:  beq v0,zero,0x8002ef54
8002eefc:  _nop
8002ef00:  lb v0,0xe(s0)
8002ef04:  nop
8002ef08:  blez v0,0x8002ef54
8002ef0c:  _lui a1,0x8006
8002ef10:  addiu a1,a1,0x5748
8002ef14:  lui a2,0x8006
8002ef18:  jal 0x80017594
8002ef1c:  _addiu a2,a2,0x5754
8002ef20:  li v0,-0x27
8002ef24:  jal 0x8004410c
8002ef28:  _sb v0,0x11(s0)
8002ef2c:  jal 0x80017160
8002ef30:  _move s1,v0
8002ef34:  andi v0,v0,0x1
8002ef38:  beq v0,zero,0x8002ef44
8002ef3c:  _li a2,0x1b
8002ef40:  li a2,0x1c
8002ef44:  lw a1,0x5f8(gp)
8002ef48:  move a0,s1
8002ef4c:  jal 0x8004483c
8002ef50:  _addiu a3,s2,0x24
8002ef54:  lw v0,0x8c(s2)
8002ef58:  nop
8002ef5c:  slti v0,v0,0x8f0
8002ef60:  beq v0,zero,0x8002ef98
8002ef64:  _nop
8002ef68:  jal 0x8004410c
8002ef6c:  _nop
8002ef70:  jal 0x80017160
8002ef74:  _move s1,v0
8002ef78:  andi v0,v0,0x1
8002ef7c:  beq v0,zero,0x8002ef88
8002ef80:  _li a2,0x1b
8002ef84:  li a2,0x1c
8002ef88:  lw a1,0x5f8(gp)
8002ef8c:  move a0,s1
8002ef90:  jal 0x8004483c
8002ef94:  _addiu a3,s2,0x24
8002ef98:  lb v0,0x11(s0)
8002ef9c:  lbu v1,0x11(s0)
8002efa0:  slti v0,v0,-0x1
8002efa4:  bne v0,zero,0x8002efc4
8002efa8:  _addiu v0,v1,0x1
8002efac:  j 0x8002efc4
8002efb0:  _li v0,0xf
8002efb4:  lb v0,0x11(s0)
8002efb8:  lbu v1,0x11(s0)
8002efbc:  blez v0,0x8002efc8
8002efc0:  _addiu v0,v1,-0x1
8002efc4:  sb v0,0x11(s0)
8002efc8:  lw ra,0x1c(sp)
8002efcc:  lw s2,0x18(sp)
8002efd0:  lw s1,0x14(sp)
8002efd4:  lw s0,0x10(sp)
8002efd8:  jr ra
8002efdc:  _addiu sp,sp,0x20
