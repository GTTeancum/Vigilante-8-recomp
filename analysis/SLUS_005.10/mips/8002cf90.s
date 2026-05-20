# addr: 0x8002cf90  name: FUN_8002cf90
8002cf90:  move t1,a0
8002cf94:  addiu a3,t1,0xa4
8002cf98:  lbu v0,0xf(a3)
8002cf9c:  lui t0,0x5555
8002cfa0:  ori t0,t0,0x5556
8002cfa4:  addiu a1,a1,0x3
8002cfa8:  addu v0,v0,a1
8002cfac:  mult v0,t0
8002cfb0:  sra a0,v0,0x1f
8002cfb4:  mfhi t2
8002cfb8:  subu a0,t2,a0
8002cfbc:  sll v1,a0,0x1
8002cfc0:  addu v1,v1,a0
8002cfc4:  subu v0,v0,v1
8002cfc8:  sb v0,0xf(a3)
8002cfcc:  andi v0,v0,0xff
8002cfd0:  addiu v0,v0,0x9
8002cfd4:  sll v0,v0,0x2
8002cfd8:  addu v0,t1,v0
8002cfdc:  lw v0,0xec(v0)
8002cfe0:  nop
8002cfe4:  bne v0,zero,0x8002d04c
8002cfe8:  _clear a2
8002cfec:  addiu a2,a2,0x1
8002cff0:  slti v0,a2,0x3
8002cff4:  beq v0,zero,0x8002d04c
8002cff8:  _nop
8002cffc:  lbu v0,0xf(a3)
8002d000:  nop
8002d004:  addu v0,v0,a1
8002d008:  mult v0,t0
8002d00c:  sra a0,v0,0x1f
8002d010:  mfhi t2
8002d014:  subu a0,t2,a0
8002d018:  sll v1,a0,0x1
8002d01c:  addu v1,v1,a0
8002d020:  subu v0,v0,v1
8002d024:  sb v0,0xf(a3)
8002d028:  andi v0,v0,0xff
8002d02c:  addiu v0,v0,0x9
8002d030:  sll v0,v0,0x2
8002d034:  addu v0,t1,v0
8002d038:  lw v0,0xec(v0)
8002d03c:  nop
8002d040:  beq v0,zero,0x8002cff0
8002d044:  _addiu a2,a2,0x1
8002d048:  addiu a2,a2,-0x1
8002d04c:  jr ra
8002d050:  _slti v0,a2,0x2
