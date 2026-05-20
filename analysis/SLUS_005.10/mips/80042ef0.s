# addr: 0x80042ef0  name: FUN_80042ef0
80042ef0:  addiu sp,sp,-0x20
80042ef4:  sw s0,0x10(sp)
80042ef8:  move s0,a2
80042efc:  move a2,a3
80042f00:  lw a3,0x30(sp)
80042f04:  sw s1,0x14(sp)
80042f08:  move s1,a0
80042f0c:  move a0,a1
80042f10:  sw ra,0x18(sp)
80042f14:  jal 0x80024d30
80042f18:  _move a1,s0
80042f1c:  move a0,s1
80042f20:  jal 0x80042e78
80042f24:  _move a1,v0
80042f28:  bne v0,zero,0x80042f48
80042f2c:  _nop
80042f30:  lw v0,0x0(s0)
80042f34:  nop
80042f38:  sw v0,0x8(s1)
80042f3c:  lw v0,0x8(s0)
80042f40:  nop
80042f44:  sw v0,0xc(s1)
80042f48:  lw ra,0x18(sp)
80042f4c:  lw s1,0x14(sp)
80042f50:  lw s0,0x10(sp)
80042f54:  jr ra
80042f58:  _addiu sp,sp,0x20
