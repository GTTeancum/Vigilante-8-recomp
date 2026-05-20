# addr: 0x801019d8  name: FUN_801019d8
801019d8:  addiu sp,sp,-0x20
801019dc:  sw s0,0x18(sp)
801019e0:  move s0,a0
801019e4:  li v0,0x3
801019e8:  beq a1,v0,0x80101a04
801019ec:  _sw ra,0x1c(sp)
801019f0:  li v0,0x8
801019f4:  beq a1,v0,0x80101a18
801019f8:  _clear v0
801019fc:  slti a0,zero,0x6a1
80101a00:  nop
80101a04:  move a0,s0
80101a08:  jal 0x8002239c
80101a0c:  _move a1,a2
80101a10:  slti a0,zero,0x689
80101a14:  nop
80101a18:  move a0,s0
80101a1c:  jal 0x80022320
80101a20:  _move a1,a2
80101a24:  beq v0,zero,0x80101a84
80101a28:  _clear v0
80101a2c:  lw v1,0x48(s0)
80101a30:  nop
80101a34:  bgez v1,0x80101a48
80101a38:  _sra v0,v1,0x10
80101a3c:  ori v0,zero,0xffff
80101a40:  addu v1,v1,v0
80101a44:  sra v0,v1,0x10
80101a48:  sh v0,0x10(sp)
80101a4c:  lw v1,0x50(s0)
80101a50:  nop
80101a54:  bgez v1,0x80101a64
80101a58:  _addiu a0,sp,0x10
80101a5c:  ori v0,zero,0xffff
80101a60:  addu v1,v1,v0
80101a64:  clear a1
80101a68:  sra v0,v1,0x10
80101a6c:  sh v0,0x12(sp)
80101a70:  li v0,0x1
80101a74:  sh v0,0x14(sp)
80101a78:  jal 0x80024718
80101a7c:  _sh v0,0x16(sp)
80101a80:  li v0,-0x1
80101a84:  lw ra,0x1c(sp)
80101a88:  lw s0,0x18(sp)
80101a8c:  jr ra
80101a90:  _addiu sp,sp,0x20
