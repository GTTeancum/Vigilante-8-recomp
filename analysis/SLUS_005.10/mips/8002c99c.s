# addr: 0x8002c99c  name: FUN_8002c99c
8002c99c:  addiu sp,sp,-0x20
8002c9a0:  sw s1,0x14(sp)
8002c9a4:  move s1,a0
8002c9a8:  addiu a1,a1,0x9
8002c9ac:  sll a1,a1,0x2
8002c9b0:  addu a1,s1,a1
8002c9b4:  sw ra,0x18(sp)
8002c9b8:  sw s0,0x10(sp)
8002c9bc:  lw a0,0xec(a1)
8002c9c0:  jal 0x8003ff28
8002c9c4:  _nop
8002c9c8:  lhu v1,0x6(s1)
8002c9cc:  move s0,v0
8002c9d0:  sh v1,0x6(s0)
8002c9d4:  lw v1,0x80(s1)
8002c9d8:  nop
8002c9dc:  bgez v1,0x8002c9e8
8002c9e0:  _nop
8002c9e4:  addiu v1,v1,0x7f
8002c9e8:  lw v0,0x88(s0)
8002c9ec:  sra v1,v1,0x7
8002c9f0:  addu v0,v0,v1
8002c9f4:  sw v0,0x88(s0)
8002c9f8:  lw v1,0x84(s1)
8002c9fc:  nop
8002ca00:  bgez v1,0x8002ca0c
8002ca04:  _nop
8002ca08:  addiu v1,v1,0x7f
8002ca0c:  lw v0,0x8c(s0)
8002ca10:  sra v1,v1,0x7
8002ca14:  addu v0,v0,v1
8002ca18:  sw v0,0x8c(s0)
8002ca1c:  lw v1,0x88(s1)
8002ca20:  nop
8002ca24:  bgez v1,0x8002ca30
8002ca28:  _nop
8002ca2c:  addiu v1,v1,0x7f
8002ca30:  lw v0,0x90(s0)
8002ca34:  sra v1,v1,0x7
8002ca38:  addu v0,v0,v1
8002ca3c:  jal 0x8004410c
8002ca40:  _sw v0,0x90(s0)
8002ca44:  lw a1,0x5f8(gp)
8002ca48:  move a0,v0
8002ca4c:  li a2,0x29
8002ca50:  jal 0x8004483c
8002ca54:  _addiu a3,s0,0x24
8002ca58:  lhu v0,0xc(s0)
8002ca5c:  nop
8002ca60:  beq v0,zero,0x8002ca80
8002ca64:  _li v0,0x6
8002ca68:  lb v1,0x8(s0)
8002ca6c:  nop
8002ca70:  beq v1,v0,0x8002ca80
8002ca74:  _lui v0,0x8004
8002ca78:  addiu v0,v0,-0x349c
8002ca7c:  sw v0,0x64(s0)
8002ca80:  lw ra,0x18(sp)
8002ca84:  lw s1,0x14(sp)
8002ca88:  lw s0,0x10(sp)
8002ca8c:  jr ra
8002ca90:  _addiu sp,sp,0x20
