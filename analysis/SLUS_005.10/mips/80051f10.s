# addr: 0x80051f10  name: SYS_OBJ_2D0C
80051f10:  and v0,v0,v1
80051f14:  bne v0,zero,0x80051eec
80051f18:  _nop
80051f1c:  lui v0,0x8006
80051f20:  lw v0,0x512c(v0)
80051f24:  nop
80051f28:  lw v0,0x0(v0)
80051f2c:  lui v1,0x400
80051f30:  and v0,v0,v1
80051f34:  beq v0,zero,0x80051eec
80051f38:  _nop
80051f3c:  lui a1,0x8005
80051f40:  addiu a1,a1,0x21c0
80051f44:  jal 0x800480b4
80051f48:  _li a0,0x2
80051f4c:  lui v0,0x8006
80051f50:  lw v0,0x501c(v0)
80051f54:  move a0,s0
80051f58:  lw v0,0x1c(v0)
80051f5c:  nop
80051f60:  jalr v0
80051f64:  _move a1,s1
80051f68:  clear v0
80051f6c:  lw ra,0x18(sp)
80051f70:  lw s1,0x14(sp)
80051f74:  lw s0,0x10(sp)
80051f78:  jr ra
80051f7c:  _addiu sp,sp,0x20
