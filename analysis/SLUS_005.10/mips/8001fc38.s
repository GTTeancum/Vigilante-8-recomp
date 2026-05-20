# addr: 0x8001fc38  name: FUN_8001fc38
8001fc38:  addiu sp,sp,-0x20
8001fc3c:  sw s0,0x10(sp)
8001fc40:  move s0,a0
8001fc44:  sw s2,0x18(sp)
8001fc48:  andi s2,a1,0xffff
8001fc4c:  sw ra,0x1c(sp)
8001fc50:  sw s1,0x14(sp)
8001fc54:  lw v0,0x0(s0)
8001fc58:  lw s1,0x34(s0)
8001fc5c:  andi v0,v0,0x4
8001fc60:  bne v0,zero,0x8001fc90
8001fc64:  _move a0,s0
8001fc68:  jal 0x8001f9cc
8001fc6c:  _move a1,s2
8001fc70:  bltz v0,0x8001fc90
8001fc74:  _nop
8001fc78:  lw a0,0x38(s0)
8001fc7c:  nop
8001fc80:  beq a0,zero,0x8001fc94
8001fc84:  _move s0,s1
8001fc88:  jal 0x8001fc38
8001fc8c:  _move a1,s2
8001fc90:  move s0,s1
8001fc94:  bne s0,zero,0x8001fc54
8001fc98:  _nop
8001fc9c:  lw ra,0x1c(sp)
8001fca0:  lw s2,0x18(sp)
8001fca4:  lw s1,0x14(sp)
8001fca8:  lw s0,0x10(sp)
8001fcac:  jr ra
8001fcb0:  _addiu sp,sp,0x20
