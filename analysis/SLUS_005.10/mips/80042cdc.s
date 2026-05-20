# addr: 0x80042cdc  name: FUN_80042cdc
80042cdc:  addiu sp,sp,-0x8
80042ce0:  sw s0,0x0(sp)
80042ce4:  move s0,a0
80042ce8:  sw s1,0x4(sp)
80042cec:  lw v0,0x8c0(gp)
80042cf0:  move s1,a1
80042cf4:  lui t6,0x7fff
80042cf8:  ori t6,t6,0xffff
80042cfc:  clear t5
80042d00:  blez v0,0x80042e64
80042d04:  _move t8,t5
80042d08:  li t9,0x1
80042d0c:  lw t4,0x8d4(gp)
80042d10:  nop
80042d14:  lw t1,0x0(t4)
80042d18:  nop
80042d1c:  lh v0,0x12(t1)
80042d20:  nop
80042d24:  blez v0,0x80042e50
80042d28:  _clear t2
80042d2c:  move t7,v0
80042d30:  move t3,t1
80042d34:  lw a3,0x1c(t3)
80042d38:  nop
80042d3c:  lw v0,0x0(a3)
80042d40:  nop
80042d44:  bne v0,t1,0x80042e40
80042d48:  _li v0,-0x1
80042d4c:  beq s1,v0,0x80042d64
80042d50:  _nop
80042d54:  lhu v0,0x8(a3)
80042d58:  nop
80042d5c:  bne v0,s1,0x80042e40
80042d60:  _nop
80042d64:  lw v0,0x4(a3)
80042d68:  lw a0,0x0(t1)
80042d6c:  lw a2,0x8(t1)
80042d70:  lw v1,0x0(v0)
80042d74:  lw a1,0x8(v0)
80042d78:  slt v1,v1,a0
80042d7c:  subu v0,t9,v1
80042d80:  sll v0,v0,0x2
80042d84:  addu v0,a3,v0
80042d88:  lw v0,0x0(v0)
80042d8c:  lw a0,0x0(s0)
80042d90:  lw v0,0x0(v0)
80042d94:  clear t0
80042d98:  subu v0,a0,v0
80042d9c:  blez v0,0x80042da8
80042da0:  _slt a1,a1,a2
80042da4:  move t0,v0
80042da8:  sll v0,v1,0x2
80042dac:  addu v0,a3,v0
80042db0:  lw v0,0x0(v0)
80042db4:  nop
80042db8:  lw v0,0x0(v0)
80042dbc:  nop
80042dc0:  subu v0,a0,v0
80042dc4:  bgez v0,0x80042dd0
80042dc8:  _clear v1
80042dcc:  move v1,v0
80042dd0:  subu v0,t9,a1
80042dd4:  sll v0,v0,0x2
80042dd8:  addu v0,a3,v0
80042ddc:  lw v0,0x0(v0)
80042de0:  lw a2,0x8(s0)
80042de4:  lw v0,0x8(v0)
80042de8:  clear a0
80042dec:  subu v0,a2,v0
80042df0:  blez v0,0x80042dfc
80042df4:  _subu t0,t0,v1
80042df8:  move a0,v0
80042dfc:  sll v0,a1,0x2
80042e00:  addu v0,a3,v0
80042e04:  lw v0,0x0(v0)
80042e08:  nop
80042e0c:  lw v0,0x8(v0)
80042e10:  nop
80042e14:  subu v0,a2,v0
80042e18:  bgez v0,0x80042e24
80042e1c:  _clear v1
80042e20:  move v1,v0
80042e24:  subu v0,a0,v1
80042e28:  addu v1,t0,v0
80042e2c:  slt v0,v1,t6
80042e30:  beq v0,zero,0x80042e40
80042e34:  _nop
80042e38:  move t6,v1
80042e3c:  move t8,a3
80042e40:  addiu t2,t2,0x1
80042e44:  slt v0,t2,t7
80042e48:  bne v0,zero,0x80042d34
80042e4c:  _addiu t3,t3,0x4
80042e50:  lw v0,0x8c0(gp)
80042e54:  addiu t5,t5,0x1
80042e58:  slt v0,t5,v0
80042e5c:  bne v0,zero,0x80042d10
80042e60:  _addiu t4,t4,0x4
80042e64:  lw s1,0x4(sp)
80042e68:  lw s0,0x0(sp)
80042e6c:  move v0,t8
80042e70:  jr ra
80042e74:  _addiu sp,sp,0x8
