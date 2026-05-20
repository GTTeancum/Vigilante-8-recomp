# addr: 0x8003fc94  name: FUN_8003fc94
8003fc94:  lw v0,0x58(a0)
8003fc98:  lhu v1,0xa(a0)
8003fc9c:  lw a3,0x0(v0)
8003fca0:  sll v0,v1,0x3
8003fca4:  subu v0,v0,v1
8003fca8:  sll v0,v0,0x2
8003fcac:  addu v0,a3,v0
8003fcb0:  lhu v1,0x36(v0)
8003fcb4:  ori v0,zero,0xffff
8003fcb8:  beq v1,v0,0x8003fd1c
8003fcbc:  _clear a2
8003fcc0:  li t1,0xff
8003fcc4:  move t0,v0
8003fcc8:  sll v0,v1,0x3
8003fccc:  subu v0,v0,v1
8003fcd0:  sll v0,v0,0x2
8003fcd4:  addu a1,a3,v0
8003fcd8:  lhu a0,0x1c(a1)
8003fcdc:  nop
8003fce0:  srl v0,a0,0x8
8003fce4:  bne v0,t1,0x8003fd00
8003fce8:  _sll v0,v1,0x3
8003fcec:  beq a0,t0,0x8003fd00
8003fcf0:  _nop
8003fcf4:  lhu v1,0x36(a1)
8003fcf8:  j 0x8003fd10
8003fcfc:  _addiu a2,a2,0x1
8003fd00:  subu v0,v0,v1
8003fd04:  sll v0,v0,0x2
8003fd08:  addu v0,a3,v0
8003fd0c:  lhu v1,0x34(v0)
8003fd10:  nop
8003fd14:  bne v1,t0,0x8003fccc
8003fd18:  _sll v0,v1,0x3
8003fd1c:  jr ra
8003fd20:  _move v0,a2
