# addr: 0x80052d54  name: memchr
80052d54:  beq a0,zero,0x80052d9c
80052d58:  _clear v0
80052d5c:  blez a2,0x80052d9c
80052d60:  _nop
80052d64:  j 0x80052d74
80052d68:  _addiu a2,a2,-0x1
80052d6c:  j 0x80052d9c
80052d70:  _addiu v0,a0,-0x1
