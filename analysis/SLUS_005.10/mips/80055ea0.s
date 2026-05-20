# addr: 0x80055ea0  name: PADPORTD_OBJ_2CC
80055ea0:  lbu v0,0x45(a0)
80055ea4:  lbu a1,0x36(a0)
80055ea8:  nop
80055eac:  beq a1,zero,0x80055ec8
80055eb0:  _addiu v1,v0,-0x3
80055eb4:  li v0,0x4d
80055eb8:  beq a1,v0,0x80055f10
80055ebc:  _nop
80055ec0:  j 0x80055f30
80055ec4:  _nop
80055ec8:  slti v0,v1,0x6
80055ecc:  beq v0,zero,0x80055ee4
80055ed0:  _addu v0,a0,v1
80055ed4:  lbu v0,0x57(v0)
80055ed8:  nop
80055edc:  beq v0,zero,0x80055f54
80055ee0:  _clear v0
80055ee4:  lbu v0,0x34(a0)
80055ee8:  nop
80055eec:  slt v0,v1,v0
80055ef0:  beq v0,zero,0x80055f54
80055ef4:  _clear v0
80055ef8:  lw v0,0x28(a0)
80055efc:  nop
80055f00:  addu v0,v0,v1
