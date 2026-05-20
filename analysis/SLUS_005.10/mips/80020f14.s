# addr: 0x80020f14  name: FUN_80020f14
80020f14:  addiu sp,sp,-0x20
80020f18:  sw s0,0x10(sp)
80020f1c:  move s0,a0
80020f20:  sw ra,0x1c(sp)
80020f24:  sw s2,0x18(sp)
80020f28:  sw s1,0x14(sp)
80020f2c:  lw v1,0x0(s0)
80020f30:  li v0,0x1
80020f34:  beq v1,v0,0x80020fb0
80020f38:  _move s1,a1
80020f3c:  beq v1,zero,0x80020f54
80020f40:  _li v0,0x2
80020f44:  beq v1,v0,0x80021024
80020f48:  _nop
80020f4c:  j 0x8002108c
80020f50:  _nop
80020f54:  lw v0,0x4(s0)
80020f58:  nop
80020f5c:  lw s0,0x0(v0)
80020f60:  nop
80020f64:  beq s0,zero,0x80020fa8
80020f68:  _nop
80020f6c:  lw a1,0x8(v0)
80020f70:  nop
80020f74:  lw v0,0x0(a1)
80020f78:  nop
80020f7c:  andi v0,v0,0x20
80020f80:  bne v0,zero,0x80020f98
80020f84:  _move v0,s0
80020f88:  jal 0x8001edb4
80020f8c:  _move a0,s1
80020f90:  bne v0,zero,0x8002101c
80020f94:  _move v0,s0
80020f98:  lw s0,0x0(s0)
80020f9c:  nop
80020fa0:  bne s0,zero,0x80020f6c
80020fa4:  _nop
80020fa8:  j 0x8002108c
80020fac:  _li v0,0x1
80020fb0:  lw v0,0x24(s1)
80020fb4:  lw v1,0x54(s1)
80020fb8:  lw a0,0x4(s0)
80020fbc:  subu v0,v0,v1
80020fc0:  slt v0,v0,a0
80020fc4:  beq v0,zero,0x80020fe0
80020fc8:  _clear s2
80020fcc:  lw a0,0x8(s0)
80020fd0:  jal 0x80020f14
80020fd4:  _move a1,s1
80020fd8:  beq v0,zero,0x8002108c
80020fdc:  _move v0,s2
80020fe0:  lw v0,0x24(s1)
80020fe4:  lw a0,0x54(s1)
80020fe8:  lw v1,0x4(s0)
80020fec:  addu v0,v0,a0
80020ff0:  slt v1,v1,v0
80020ff4:  beq v1,zero,0x80021010
80020ff8:  _nop
80020ffc:  lw a0,0xc(s0)
80021000:  jal 0x80020f14
80021004:  _move a1,s1
80021008:  beq v0,zero,0x8002108c
8002100c:  _move v0,s2
80021010:  li s2,0x1
80021014:  j 0x8002108c
80021018:  _move v0,s2
8002101c:  j 0x8002108c
80021020:  _clear v0
80021024:  lw v0,0x2c(s1)
80021028:  lw v1,0x54(s1)
8002102c:  lw a0,0x4(s0)
80021030:  subu v0,v0,v1
80021034:  slt v0,v0,a0
80021038:  beq v0,zero,0x80021054
8002103c:  _clear s2
80021040:  lw a0,0x8(s0)
80021044:  jal 0x80020f14
80021048:  _move a1,s1
8002104c:  beq v0,zero,0x8002108c
80021050:  _move v0,s2
80021054:  lw v0,0x2c(s1)
80021058:  lw a0,0x54(s1)
8002105c:  lw v1,0x4(s0)
80021060:  addu v0,v0,a0
80021064:  slt v1,v1,v0
80021068:  beq v1,zero,0x80021084
8002106c:  _nop
80021070:  lw a0,0xc(s0)
80021074:  jal 0x80020f14
80021078:  _move a1,s1
8002107c:  beq v0,zero,0x8002108c
80021080:  _move v0,s2
80021084:  li s2,0x1
80021088:  move v0,s2
8002108c:  lw ra,0x1c(sp)
80021090:  lw s2,0x18(sp)
80021094:  lw s1,0x14(sp)
80021098:  lw s0,0x10(sp)
8002109c:  jr ra
800210a0:  _addiu sp,sp,0x20
