# addr: 0x80043ce0  name: FUN_80043ce0
80043ce0:  lw v1,0x5ac(gp)
80043ce4:  addiu sp,sp,-0x20
80043ce8:  li v0,0x5
80043cec:  sw ra,0x18(sp)
80043cf0:  sb v0,0x10(sp)
80043cf4:  sb a0,0x8f0(gp)
80043cf8:  sltiu v1,v1,0x3
80043cfc:  bne v1,zero,0x80043d0c
80043d00:  _nop
80043d04:  j 0x80043d10
80043d08:  _addiu a0,a0,0x2
80043d0c:  addiu a0,a0,0x1
80043d10:  lui v1,0x800a
80043d14:  addiu v1,v1,0x3090
80043d18:  sll v0,a0,0x2
80043d1c:  addiu a0,v1,0x4
80043d20:  addu a0,v0,a0
80043d24:  addu v0,v0,v1
80043d28:  lui t2,0x8006
80043d2c:  addiu t2,t2,0x5be0
80043d30:  lwl a3,0x3(v0)
80043d34:  lwr a3,0x0(v0)
80043d38:  nop
80043d3c:  swl a3,0x3(t2)
80043d40:  swr a3,0x0(t2)
80043d44:  jal 0x80049534
80043d48:  _nop
80043d4c:  li a0,0xe
80043d50:  addiu a1,sp,0x10
80043d54:  addiu v0,v0,-0x96
80043d58:  sw v0,0x8e0(gp)
80043d5c:  jal 0x80048fd0
80043d60:  _clear a2
80043d64:  li a0,0x3
80043d68:  lui a1,0x8006
80043d6c:  addiu a1,a1,0x5be0
80043d70:  jal 0x80048fd0
80043d74:  _clear a2
80043d78:  lui a0,0x8004
80043d7c:  jal 0x80048fbc
80043d80:  _addiu a0,a0,0x3c34
80043d84:  lw ra,0x18(sp)
80043d88:  nop
80043d8c:  jr ra
80043d90:  _addiu sp,sp,0x20
