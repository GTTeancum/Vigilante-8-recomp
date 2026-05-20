# addr: 0x8001ecc4  name: FUN_8001ecc4
8001ecc4:  addiu sp,sp,-0x40
8001ecc8:  sw ra,0x3c(sp)
8001eccc:  sw s2,0x38(sp)
8001ecd0:  sw s1,0x34(sp)
8001ecd4:  sw s0,0x30(sp)
8001ecd8:  lw s0,0x38(a1)
8001ecdc:  move s1,a0
8001ece0:  beq s0,zero,0x8001ed98
8001ece4:  _move s2,a2
8001ece8:  lw v0,0x5c(s0)
8001ecec:  nop
8001ecf0:  beq v0,zero,0x8001ed50
8001ecf4:  _nop
8001ecf8:  lw v0,0x0(s0)
8001ecfc:  nop
8001ed00:  andi v0,v0,0x20
8001ed04:  bne v0,zero,0x8001ed50
8001ed08:  _move a0,s2
8001ed0c:  addiu a1,s0,0x10
8001ed10:  jal 0x8004cf04
8001ed14:  _addiu a2,sp,0x10
8001ed18:  move a0,s1
8001ed1c:  move a1,s0
8001ed20:  addiu a2,s1,0x10
8001ed24:  jal 0x8001e9a0
8001ed28:  _addiu a3,sp,0x10
8001ed2c:  bne v0,zero,0x8001ed9c
8001ed30:  _nop
8001ed34:  lw v0,0x0(s0)
8001ed38:  nop
8001ed3c:  andi v0,v0,0x800
8001ed40:  beq v0,zero,0x8001ed88
8001ed44:  _move a0,s1
8001ed48:  j 0x8001ed78
8001ed4c:  _move a1,s0
8001ed50:  lw v0,0x0(s0)
8001ed54:  nop
8001ed58:  andi v0,v0,0x800
8001ed5c:  beq v0,zero,0x8001ed88
8001ed60:  _move a0,s2
8001ed64:  addiu a1,s0,0x10
8001ed68:  jal 0x8004cf04
8001ed6c:  _addiu a2,sp,0x10
8001ed70:  move a0,s1
8001ed74:  move a1,s0
8001ed78:  jal 0x8001ecc4
8001ed7c:  _addiu a2,sp,0x10
8001ed80:  bne v0,zero,0x8001ed9c
8001ed84:  _nop
8001ed88:  lw s0,0x34(s0)
8001ed8c:  nop
8001ed90:  bne s0,zero,0x8001ece8
8001ed94:  _nop
8001ed98:  clear v0
8001ed9c:  lw ra,0x3c(sp)
8001eda0:  lw s2,0x38(sp)
8001eda4:  lw s1,0x34(sp)
8001eda8:  lw s0,0x30(sp)
8001edac:  jr ra
8001edb0:  _addiu sp,sp,0x40
