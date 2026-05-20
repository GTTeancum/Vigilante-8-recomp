# addr: 0x80016a20  name: FUN_80016a20
80016a20:  addiu sp,sp,-0x18
80016a24:  move a1,a0
80016a28:  sw ra,0x14(sp)
80016a2c:  jal 0x800171d4
80016a30:  _sw s0,0x10(sp)
80016a34:  sra a0,v1,0x0
80016a38:  sra a1,v1,0x1f
80016a3c:  gte_ldLZCS a0
80016a40:  nop
80016a44:  nop
80016a48:  gte_stLZCR a0
80016a4c:  li s0,0x23
80016a50:  subu s0,s0,a0
80016a54:  sra s0,s0,0x1
80016a58:  sll a0,s0,0x1
80016a5c:  sll a2,a0,0x1a
80016a60:  bgez a2,0x80016a74
80016a64:  _nop
80016a68:  srav t0,v1,a0
80016a6c:  bgez zero,0x80016a8c
80016a70:  _sra t1,v1,0x1f
80016a74:  beq a2,zero,0x80016a88
80016a78:  _srlv t0,v0,a0
80016a7c:  subu a2,zero,a0
80016a80:  sllv a2,v1,a2
80016a84:  or t0,t0,a2
80016a88:  srav t1,v1,a0
80016a8c:  move a0,t0
80016a90:  jal 0x8004c6e4
80016a94:  _move a1,t1
80016a98:  lw ra,0x14(sp)
80016a9c:  sllv v0,v0,s0
80016aa0:  lw s0,0x10(sp)
80016aa4:  jr ra
80016aa8:  _addiu sp,sp,0x18
