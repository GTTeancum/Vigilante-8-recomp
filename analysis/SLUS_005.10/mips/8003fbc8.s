# addr: 0x8003fbc8  name: FUN_8003fbc8
8003fbc8:  lw v0,0x58(a0)
8003fbcc:  lhu v1,0xa(a0)
8003fbd0:  lw a1,0x0(v0)
8003fbd4:  sll v0,v1,0x3
8003fbd8:  subu v0,v0,v1
8003fbdc:  sll v0,v0,0x2
8003fbe0:  addu v0,a1,v0
8003fbe4:  lhu v1,0x36(v0)
8003fbe8:  ori v0,zero,0xffff
8003fbec:  beq v1,v0,0x8003fc48
8003fbf0:  _li a3,0xff
8003fbf4:  move a2,v0
8003fbf8:  sll v0,v1,0x3
8003fbfc:  subu v0,v0,v1
8003fc00:  sll v0,v0,0x2
8003fc04:  addu v0,a1,v0
8003fc08:  lhu a0,0x1c(v0)
8003fc0c:  nop
8003fc10:  srl v0,a0,0x8
8003fc14:  bne v0,a3,0x8003fc2c
8003fc18:  _sll v0,v1,0x3
8003fc1c:  beq a0,a2,0x8003fc30
8003fc20:  _subu v0,v0,v1
8003fc24:  jr ra
8003fc28:  _move v0,v1
8003fc2c:  subu v0,v0,v1
8003fc30:  sll v0,v0,0x2
8003fc34:  addu v0,a1,v0
8003fc38:  lhu v1,0x34(v0)
8003fc3c:  nop
8003fc40:  bne v1,a2,0x8003fbfc
8003fc44:  _sll v0,v1,0x3
8003fc48:  jr ra
8003fc4c:  _clear v0
