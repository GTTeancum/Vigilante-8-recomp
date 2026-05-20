# addr: 0x80049c48  name: BIOS_OBJ_694
80049c48:  bne v0,zero,0x80049d60
80049c4c:  _li v0,-0x1
80049c50:  jal 0x800481a8
80049c54:  _nop
80049c58:  beq v0,zero,0x80049d00
80049c5c:  _nop
80049c60:  lui v0,0x8006
80049c64:  lw v0,0x344(v0)
80049c68:  nop
80049c6c:  lbu v0,0x0(v0)
80049c70:  nop
80049c74:  andi s1,v0,0x3
