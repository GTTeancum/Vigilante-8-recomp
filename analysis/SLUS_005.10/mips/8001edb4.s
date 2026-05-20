# addr: 0x8001edb4  name: FUN_8001edb4
8001edb4:  addiu sp,sp,-0x20
8001edb8:  sw s2,0x18(sp)
8001edbc:  move s2,a0
8001edc0:  sw ra,0x1c(sp)
8001edc4:  sw s1,0x14(sp)
8001edc8:  sw s0,0x10(sp)
8001edcc:  lh v1,0x6(s2)
8001edd0:  move s1,a1
8001edd4:  lh v0,0x6(s1)
8001edd8:  nop
8001eddc:  beq v1,v0,0x8001ef58
8001ede0:  _clear t0
8001ede4:  lw a1,0x54(s2)
8001ede8:  lw a0,0x54(s1)
8001edec:  lw v0,0x24(s2)
8001edf0:  lw v1,0x24(s1)
8001edf4:  addiu a2,s2,0x24
8001edf8:  addu a1,a1,a0
8001edfc:  subu v0,v0,v1
8001ee00:  bgez v0,0x8001ee0c
8001ee04:  _nop
8001ee08:  subu v0,zero,v0
8001ee0c:  slt v0,v0,a1
8001ee10:  beq v0,zero,0x8001ee60
8001ee14:  _addiu a3,s1,0x24
8001ee18:  lw v0,0x4(a2)
8001ee1c:  lw v1,0x4(a3)
8001ee20:  nop
8001ee24:  subu v0,v0,v1
8001ee28:  bgez v0,0x8001ee34
8001ee2c:  _nop
8001ee30:  subu v0,zero,v0
8001ee34:  slt v0,v0,a1
8001ee38:  beq v0,zero,0x8001ee60
8001ee3c:  _nop
8001ee40:  lw v0,0x8(a2)
8001ee44:  lw v1,0x8(a3)
8001ee48:  nop
8001ee4c:  subu v0,v0,v1
8001ee50:  bgez v0,0x8001ee5c
8001ee54:  _nop
8001ee58:  subu v0,zero,v0
8001ee5c:  slt t0,v0,a1
8001ee60:  beq t0,zero,0x8001ef5c
8001ee64:  _clear v0
8001ee68:  lw v0,0x0(s1)
8001ee6c:  nop
8001ee70:  andi v0,v0,0x40
8001ee74:  beq v0,zero,0x8001ee98
8001ee78:  _nop
8001ee7c:  lw v0,0x74(s2)
8001ee80:  nop
8001ee84:  bne v0,zero,0x8001ee94
8001ee88:  _nop
8001ee8c:  j 0x8001ee98
8001ee90:  _sw s1,0x74(s2)
8001ee94:  sw s1,0x78(s2)
8001ee98:  lw v0,0x0(s1)
8001ee9c:  nop
8001eea0:  andi v0,v0,0x800
8001eea4:  beq v0,zero,0x8001eec4
8001eea8:  _move a0,s2
8001eeac:  move a1,s1
8001eeb0:  jal 0x8001ecc4
8001eeb4:  _addiu a2,s1,0x10
8001eeb8:  move s0,v0
8001eebc:  bne s0,zero,0x8001eee4
8001eec0:  _nop
8001eec4:  move a0,s2
8001eec8:  move a1,s1
8001eecc:  addiu a2,s2,0x10
8001eed0:  jal 0x8001e9a0
8001eed4:  _addiu a3,s1,0x10
8001eed8:  move s0,v0
8001eedc:  beq s0,zero,0x8001ef5c
8001eee0:  _clear v0
8001eee4:  sw s1,0x0(s0)
8001eee8:  move a0,s2
8001eeec:  li a1,0x3
8001eef0:  jal 0x8001e120
8001eef4:  _move a2,s0
8001eef8:  move t1,v0
8001eefc:  beq t1,zero,0x8001ef0c
8001ef00:  _li v0,-0x1
8001ef04:  bne t1,v0,0x8001ef5c
8001ef08:  _srl v0,t1,0x1f
8001ef0c:  lw a3,0x4(s0)
8001ef10:  lw v0,0x8(s0)
8001ef14:  lw t0,0xc(s0)
8001ef18:  lw v1,0x10(s0)
8001ef1c:  move a0,s1
8001ef20:  li a1,0x3
8001ef24:  move a2,s0
8001ef28:  srl s0,t1,0x1f
8001ef2c:  sw s2,0x0(a2)
8001ef30:  sw v0,0x4(a2)
8001ef34:  sw a3,0x8(a2)
8001ef38:  sw v1,0xc(a2)
8001ef3c:  jal 0x8001e120
8001ef40:  _sw t0,0x10(a2)
8001ef44:  bgez v0,0x8001ef5c
8001ef48:  _move v0,s0
8001ef4c:  ori s0,s0,0x2
8001ef50:  j 0x8001ef5c
8001ef54:  _move v0,s0
8001ef58:  clear v0
8001ef5c:  lw ra,0x1c(sp)
8001ef60:  lw s2,0x18(sp)
8001ef64:  lw s1,0x14(sp)
8001ef68:  lw s0,0x10(sp)
8001ef6c:  jr ra
8001ef70:  _addiu sp,sp,0x20
