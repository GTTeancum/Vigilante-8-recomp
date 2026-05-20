# addr: 0x80055d34  name: PADPORTD_OBJ_160
80055d34:  lbu v0,0x49(a0)
80055d38:  nop
80055d3c:  beq v0,zero,0x80055d94
80055d40:  _nop
80055d44:  addiu v1,a0,0x5d
80055d48:  li a1,0xff
80055d4c:  li v0,0x5
80055d50:  sb zero,0x49(a0)
80055d54:  sb zero,0x46(a0)
80055d58:  sh zero,0xe6(a0)
80055d5c:  sw zero,0x14(a0)
80055d60:  sw zero,0x18(a0)
80055d64:  sb zero,0xe3(a0)
80055d68:  sb zero,0xe4(a0)
80055d6c:  sh zero,0xe6(a0)
80055d70:  sb zero,0xe9(a0)
80055d74:  sb zero,0xea(a0)
80055d78:  sw zero,0x0(a0)
80055d7c:  sw zero,0x4(a0)
80055d80:  sw zero,0x8(a0)
80055d84:  sb a1,0x0(v1)
80055d88:  addiu v0,v0,-0x1
80055d8c:  bgez v0,0x80055d84
80055d90:  _addiu v1,v1,0x1
80055d94:  jr ra
80055d98:  _nop
