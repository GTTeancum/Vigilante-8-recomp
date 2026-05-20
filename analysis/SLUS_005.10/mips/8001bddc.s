# addr: 0x8001bddc  name: FUN_8001bddc
8001bddc:  addiu sp,sp,-0x18
8001bde0:  sw s0,0x10(sp)
8001bde4:  move s0,a0
8001bde8:  beq s0,zero,0x8001be4c
8001bdec:  _sw ra,0x14(sp)
8001bdf0:  lw v0,0x4(gp)
8001bdf4:  nop
8001bdf8:  sll v0,v0,0x2
8001bdfc:  addu v0,s0,v0
8001be00:  lw a0,0x1c(v0)
8001be04:  nop
8001be08:  beq a0,zero,0x8001be18
8001be0c:  _nop
8001be10:  jal 0x800118b4
8001be14:  _nop
8001be18:  lw v1,0x4(gp)
8001be1c:  li v0,0x1
8001be20:  subu v0,v0,v1
8001be24:  sll v0,v0,0x2
8001be28:  addu v0,s0,v0
8001be2c:  lw a0,0x1c(v0)
8001be30:  nop
8001be34:  beq a0,zero,0x8001be44
8001be38:  _nop
8001be3c:  jal 0x80045088
8001be40:  _nop
8001be44:  jal 0x80045088
8001be48:  _move a0,s0
8001be4c:  lw ra,0x14(sp)
8001be50:  lw s0,0x10(sp)
8001be54:  jr ra
8001be58:  _addiu sp,sp,0x18
