# addr: 0x80044fbc  name: FUN_80044fbc
80044fbc:  beq a0,zero,0x80044ffc
80044fc0:  _li v0,-0x8
80044fc4:  beq a1,zero,0x80044ffc
80044fc8:  _and a1,a1,v0
80044fcc:  addu v0,a0,a1
80044fd0:  sw a0,-0x8(v0)
80044fd4:  sw zero,-0x4(v0)
80044fd8:  addiu v0,v0,-0x8
80044fdc:  sw v0,0x0(a0)
80044fe0:  srl v0,a1,0x3
80044fe4:  addiu v0,v0,-0x1
80044fe8:  lui at,0x8006
80044fec:  sw a0,-0x12b4(at)
80044ff0:  lui at,0x8006
80044ff4:  sw a0,-0x12b0(at)
80044ff8:  sw v0,0x4(a0)
80044ffc:  jr ra
80045000:  _nop
