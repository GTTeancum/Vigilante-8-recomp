# addr: 0x80049ec8  name: BIOS_OBJ_914
80049ec8:  bne v0,zero,0x8004a028
80049ecc:  _li v0,-0x1
80049ed0:  jal 0x800481a8
80049ed4:  _nop
80049ed8:  beq v0,zero,0x80049f80
80049edc:  _nop
80049ee0:  lui v0,0x8006
80049ee4:  lw v0,0x344(v0)
80049ee8:  nop
80049eec:  lbu v0,0x0(v0)
80049ef0:  nop
80049ef4:  andi s1,v0,0x3
