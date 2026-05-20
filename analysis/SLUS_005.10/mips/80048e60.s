# addr: 0x80048e60  name: CdReset
80048e60:  addiu sp,sp,-0x18
80048e64:  sw s0,0x10(sp)
80048e68:  move s0,a0
80048e6c:  li v0,0x2
80048e70:  bne s0,v0,0x80048e88
80048e74:  _sw ra,0x14(sp)
80048e78:  jal 0x8004a6b0
80048e7c:  _nop
80048e80:  j 0x80048ebc
80048e84:  _li v0,0x1
80048e88:  jal 0x8004a6fc
80048e8c:  _nop
80048e90:  bne v0,zero,0x80048ebc
80048e94:  _clear v0
80048e98:  li v0,0x1
80048e9c:  bne s0,v0,0x80048ebc
80048ea0:  _nop
80048ea4:  jal 0x8004a5c0
80048ea8:  _nop
80048eac:  move v1,v0
80048eb0:  bne v1,zero,0x80048ebc
80048eb4:  _clear v0
80048eb8:  li v0,0x1
