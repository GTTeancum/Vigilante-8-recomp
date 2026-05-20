# addr: 0x8001ec48  name: FUN_8001ec48
8001ec48:  addiu sp,sp,-0x20
8001ec4c:  sw s2,0x18(sp)
8001ec50:  move s2,a0
8001ec54:  sw ra,0x1c(sp)
8001ec58:  sw s1,0x14(sp)
8001ec5c:  sw s0,0x10(sp)
8001ec60:  lw s0,0x38(s2)
8001ec64:  nop
8001ec68:  beq s0,zero,0x8001ec88
8001ec6c:  _clear s1
8001ec70:  jal 0x8001ec48
8001ec74:  _move a0,s0
8001ec78:  lw s0,0x34(s0)
8001ec7c:  nop
8001ec80:  bne s0,zero,0x8001ec70
8001ec84:  _or s1,s1,v0
8001ec88:  beq s1,zero,0x8001eca0
8001ec8c:  _nop
8001ec90:  lw v0,0x0(s2)
8001ec94:  nop
8001ec98:  ori v0,v0,0x800
8001ec9c:  sw v0,0x0(s2)
8001eca0:  lw v0,0x5c(s2)
8001eca4:  lw ra,0x1c(sp)
8001eca8:  lw s2,0x18(sp)
8001ecac:  lw s0,0x10(sp)
8001ecb0:  sltu v0,zero,v0
8001ecb4:  or v0,s1,v0
8001ecb8:  lw s1,0x14(sp)
8001ecbc:  jr ra
8001ecc0:  _addiu sp,sp,0x20
