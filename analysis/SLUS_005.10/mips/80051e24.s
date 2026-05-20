# addr: 0x80051e24  name: SYS_OBJ_2C20
80051e24:  and v0,v0,v1
80051e28:  bne v0,zero,0x80051e00
80051e2c:  _nop
80051e30:  lui v0,0x8006
80051e34:  lw v0,0x512c(v0)
80051e38:  nop
80051e3c:  lw v0,0x0(v0)
80051e40:  lui v1,0x400
80051e44:  and v0,v0,v1
80051e48:  beq v0,zero,0x80051e00
80051e4c:  _nop
80051e50:  lui a1,0x8005
80051e54:  addiu a1,a1,0x21c0
80051e58:  jal 0x800480b4
80051e5c:  _li a0,0x2
80051e60:  lui v0,0x8006
80051e64:  lw v0,0x501c(v0)
80051e68:  move a0,s0
80051e6c:  lw v0,0x20(v0)
80051e70:  nop
80051e74:  jalr v0
80051e78:  _move a1,s1
80051e7c:  clear v0
80051e80:  lw ra,0x18(sp)
80051e84:  lw s1,0x14(sp)
80051e88:  lw s0,0x10(sp)
80051e8c:  jr ra
80051e90:  _addiu sp,sp,0x20
