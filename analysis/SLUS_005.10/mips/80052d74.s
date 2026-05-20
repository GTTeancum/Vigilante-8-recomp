# addr: 0x80052d74  name: MEMCHR_OBJ_20
80052d74:  bltz a2,0x80052d9c
80052d78:  _clear v0
80052d7c:  andi a1,a1,0xff
80052d80:  lbu v0,0x0(a0)
80052d84:  nop
80052d88:  beq v0,a1,0x80052d6c
80052d8c:  _addiu a0,a0,0x1
80052d90:  addiu a2,a2,-0x1
80052d94:  bgez a2,0x80052d80
80052d98:  _clear v0
