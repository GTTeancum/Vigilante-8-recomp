# addr: 0x80032e48  name: FUN_80032e48
80032e48:  addiu sp,sp,-0x40
80032e4c:  sw s3,0x34(sp)
80032e50:  move s3,a0
80032e54:  sw s1,0x2c(sp)
80032e58:  move s1,a1
80032e5c:  move a0,s1
80032e60:  move a1,s3
80032e64:  sll a2,a2,0x10
80032e68:  sra a2,a2,0x10
80032e6c:  li a3,0x98
80032e70:  sw ra,0x38(sp)
80032e74:  sw s2,0x30(sp)
80032e78:  sw s0,0x28(sp)
80032e7c:  jal 0x80031300
80032e80:  _sw zero,0x10(sp)
80032e84:  move s0,v0
80032e88:  lh v0,0x12(s0)
80032e8c:  nop
80032e90:  sll v0,v0,0x5
80032e94:  sw v0,0x18(sp)
80032e98:  lh v0,0x18(s0)
80032e9c:  addiu s2,sp,0x18
80032ea0:  sll v0,v0,0x5
80032ea4:  sw v0,0x4(s2)
80032ea8:  lh v0,0x1e(s0)
80032eac:  lui v1,0x80
80032eb0:  ori v1,v1,0x84
80032eb4:  move a0,s0
80032eb8:  sll v0,v0,0x5
80032ebc:  sw v0,0x8(s2)
80032ec0:  lui v0,0x8003
80032ec4:  addiu v0,v0,0x2aa4
80032ec8:  sw v1,0x0(s0)
80032ecc:  jal 0x800202f4
80032ed0:  _sw v0,0x64(s0)
80032ed4:  lw a0,0x80(s1)
80032ed8:  nop
80032edc:  bgez a0,0x80032ee8
80032ee0:  _nop
80032ee4:  addiu a0,a0,0x7f
80032ee8:  lh v0,0x12(s0)
80032eec:  nop
80032ef0:  sll v1,v0,0x5
80032ef4:  subu v1,v1,v0
80032ef8:  sll v1,v1,0x2
80032efc:  addu v1,v1,v0
80032f00:  sll v0,v1,0x3
80032f04:  subu v0,v0,v1
80032f08:  sll v0,v0,0x1
80032f0c:  bgez v0,0x80032f18
80032f10:  _sra v1,a0,0x7
80032f14:  addiu v0,v0,0xfff
80032f18:  sra v0,v0,0xc
80032f1c:  subu v0,v1,v0
80032f20:  sw v0,0x88(s0)
80032f24:  lw a0,0x84(s1)
80032f28:  nop
80032f2c:  bgez a0,0x80032f38
80032f30:  _nop
80032f34:  addiu a0,a0,0x7f
80032f38:  lh v0,0x18(s0)
80032f3c:  nop
80032f40:  sll v1,v0,0x5
80032f44:  subu v1,v1,v0
80032f48:  sll v1,v1,0x2
80032f4c:  addu v1,v1,v0
80032f50:  sll v0,v1,0x3
80032f54:  subu v0,v0,v1
80032f58:  sll v0,v0,0x1
80032f5c:  bgez v0,0x80032f68
80032f60:  _sra v1,a0,0x7
80032f64:  addiu v0,v0,0xfff
80032f68:  sra v0,v0,0xc
80032f6c:  subu v0,v1,v0
80032f70:  sw v0,0x8c(s0)
80032f74:  lw a0,0x88(s1)
80032f78:  nop
80032f7c:  bgez a0,0x80032f88
80032f80:  _nop
80032f84:  addiu a0,a0,0x7f
80032f88:  lh v0,0x1e(s0)
80032f8c:  nop
80032f90:  sll v1,v0,0x5
80032f94:  subu v1,v1,v0
80032f98:  sll v1,v1,0x2
80032f9c:  addu v1,v1,v0
80032fa0:  sll v0,v1,0x3
80032fa4:  subu v0,v0,v1
80032fa8:  sll v0,v0,0x1
80032fac:  bgez v0,0x80032fb8
80032fb0:  _sra v1,a0,0x7
80032fb4:  addiu v0,v0,0xfff
80032fb8:  sra v0,v0,0xc
80032fbc:  subu v0,v1,v0
80032fc0:  sw v0,0x90(s0)
80032fc4:  lw v0,0xe4(s1)
80032fc8:  nop
80032fcc:  bne v0,zero,0x80032fd8
80032fd0:  _move a0,s1
80032fd4:  move v0,s1
80032fd8:  move a1,s2
80032fdc:  addiu a2,s0,0x48
80032fe0:  jal 0x800176f8
80032fe4:  _sw v0,0x84(s0)
80032fe8:  lhu v0,0xc(s3)
80032fec:  nop
80032ff0:  addiu v0,v0,-0x1
80032ff4:  sh v0,0xc(s3)
80032ff8:  andi v0,v0,0xffff
80032ffc:  bne v0,zero,0x8003300c
80033000:  _nop
80033004:  jal 0x8002cb7c
80033008:  _move a0,s3
8003300c:  lw ra,0x38(sp)
80033010:  lw s3,0x34(sp)
80033014:  lw s2,0x30(sp)
80033018:  lw s1,0x2c(sp)
8003301c:  move v0,s0
80033020:  lw s0,0x28(sp)
80033024:  jr ra
80033028:  _addiu sp,sp,0x40
