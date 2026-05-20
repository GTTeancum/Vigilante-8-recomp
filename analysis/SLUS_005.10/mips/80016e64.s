# addr: 0x80016e64  name: FUN_80016e64
80016e64:  addiu sp,sp,-0x18
80016e68:  sw s0,0x10(sp)
80016e6c:  move s0,a0
80016e70:  sw ra,0x14(sp)
80016e74:  lh v0,0x0(s0)
80016e78:  nop
80016e7c:  mult v0,v0
80016e80:  lh v0,0xc(s0)
80016e84:  mflo v1
80016e88:  nop
80016e8c:  nop
80016e90:  mult v0,v0
80016e94:  mflo a1
80016e98:  jal 0x8004c6e4
80016e9c:  _addu a0,v1,a1
80016ea0:  move a1,v0
80016ea4:  lui v0,0x100
80016ea8:  div v0,a1
80016eac:  mflo a0
80016eb0:  lh v0,0x0(s0)
80016eb4:  nop
80016eb8:  mult v0,a0
80016ebc:  mflo v1
80016ec0:  bgez v1,0x80016ecc
80016ec4:  _nop
80016ec8:  addiu v1,v1,0xfff
80016ecc:  lh v0,0xc(s0)
80016ed0:  nop
80016ed4:  mult v0,a0
80016ed8:  mflo a0
80016edc:  bgez a0,0x80016ee8
80016ee0:  _sra a2,v1,0xc
80016ee4:  addiu a0,a0,0xfff
80016ee8:  sh a1,0x10(s0)
80016eec:  lh a1,0x6(s0)
80016ef0:  nop
80016ef4:  mult a2,a1
80016ef8:  lh v0,0xa(s0)
80016efc:  mflo v1
80016f00:  sra a0,a0,0xc
80016f04:  nop
80016f08:  mult a0,v0
80016f0c:  mflo t0
80016f10:  subu v1,v1,t0
80016f14:  bgez v1,0x80016f20
80016f18:  _sh zero,0xc(s0)
80016f1c:  addiu v1,v1,0xfff
80016f20:  mult a0,a1
80016f24:  lh v0,0xa(s0)
80016f28:  mflo a1
80016f2c:  nop
80016f30:  nop
80016f34:  mult a2,v0
80016f38:  sra v0,v1,0xc
80016f3c:  mflo t0
80016f40:  addu v1,a1,t0
80016f44:  bgez v1,0x80016f50
80016f48:  _sh v0,0x6(s0)
80016f4c:  addiu v1,v1,0xfff
80016f50:  lh v0,0x0(s0)
80016f54:  nop
80016f58:  mult a2,v0
80016f5c:  lh v0,0x4(s0)
80016f60:  mflo a1
80016f64:  nop
80016f68:  nop
80016f6c:  mult a0,v0
80016f70:  sra v0,v1,0xc
80016f74:  mflo a2
80016f78:  subu v1,a1,a2
80016f7c:  bgez v1,0x80016f88
80016f80:  _sh v0,0xa(s0)
80016f84:  addiu v1,v1,0xfff
80016f88:  move v0,s0
80016f8c:  sra v1,v1,0xc
80016f90:  sh v1,0x0(v0)
80016f94:  sh zero,0x4(v0)
80016f98:  lw ra,0x14(sp)
80016f9c:  lw s0,0x10(sp)
80016fa0:  jr ra
80016fa4:  _addiu sp,sp,0x18
