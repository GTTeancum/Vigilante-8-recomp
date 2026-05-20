# addr: 0x80047fbc  name: VSYNC_OBJ_178
80047fbc:  addiu sp,sp,-0x20
80047fc0:  sll a1,a1,0xf
80047fc4:  sw a1,0x10(sp)
80047fc8:  lui v0,0x8006
80047fcc:  lw v0,-0x4c(v0)
80047fd0:  nop
80047fd4:  slt v0,v0,a0
80047fd8:  beq v0,zero,0x80048044
80047fdc:  _sw ra,0x18(sp)
80047fe0:  li v1,-0x1
80047fe4:  lw v0,0x10(sp)
80047fe8:  nop
80047fec:  addiu v0,v0,-0x1
80047ff0:  sw v0,0x10(sp)
80047ff4:  lw v0,0x10(sp)
80047ff8:  nop
80047ffc:  bne v0,v1,0x8004802c
80048000:  _nop
80048004:  lui a0,0x8001
80048008:  jal 0x80053884
8004800c:  _addiu a0,a0,0xd64
80048010:  jal 0x80053a54
80048014:  _clear a0
80048018:  li a0,0x3
8004801c:  jal 0x80053a64
80048020:  _clear a1
80048024:  j 0x80048044
80048028:  _nop
8004802c:  lui v0,0x8006
80048030:  lw v0,-0x4c(v0)
80048034:  nop
80048038:  slt v0,v0,a0
8004803c:  bne v0,zero,0x80047fe4
80048040:  _nop
