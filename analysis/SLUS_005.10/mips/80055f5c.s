# addr: 0x80055f5c  name: PADPORTD_OBJ_388
80055f5c:  addiu sp,sp,-0x18
80055f60:  sw s0,0x10(sp)
80055f64:  move s0,a0
80055f68:  addiu a0,s0,0x57
80055f6c:  sw ra,0x14(sp)
80055f70:  jal 0x80044f64
80055f74:  _li a1,0x6
80055f78:  lhu v0,0xe6(s0)
80055f7c:  nop
80055f80:  beq v0,zero,0x800560d4
80055f84:  _nop
80055f88:  lw v0,0x28(s0)
80055f8c:  nop
80055f90:  beq v0,zero,0x800560d4
80055f94:  _nop
80055f98:  lbu v0,0x34(s0)
80055f9c:  nop
80055fa0:  sltiu v0,v0,0x7
80055fa4:  beq v0,zero,0x80055fb0
80055fa8:  _li t1,0x6
80055fac:  lbu t1,0x34(s0)
80055fb0:  lbu v0,0xe9(s0)
80055fb4:  nop
80055fb8:  beq v0,zero,0x800561c8
80055fbc:  _clear t0
80055fc0:  li t3,0x1
80055fc4:  clear t2
80055fc8:  lw v0,0x4(s0)
80055fcc:  clear a2
80055fd0:  addu v0,t2,v0
80055fd4:  lbu v0,0x2(v0)
80055fd8:  nop
80055fdc:  beq v0,zero,0x80055fe8
80055fe0:  _li a3,0x1
80055fe4:  li a3,0xff
80055fe8:  addiu a1,s0,0x5d
80055fec:  lw a0,0x28(s0)
80055ff0:  beq t1,zero,0x80056030
80055ff4:  _clear v1
80055ff8:  lbu v0,0x0(a1)
80055ffc:  nop
80056000:  bne v0,t0,0x8005601c
80056004:  _nop
80056008:  lbu v0,0x0(a0)
8005600c:  nop
80056010:  and v0,v0,a3
80056014:  bne v0,zero,0x80056070
80056018:  _nop
8005601c:  addiu a1,a1,0x1
80056020:  addiu v1,v1,0x1
80056024:  slt v0,v1,t1
80056028:  bne v0,zero,0x80055ff8
8005602c:  _addiu a0,a0,0x1
