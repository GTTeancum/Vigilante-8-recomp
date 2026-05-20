# addr: 0x800495b4  name: BIOS_OBJ_0
800495b4:  addiu sp,sp,-0x30
800495b8:  lui v1,0x8006
800495bc:  lw v1,0x344(v1)
800495c0:  li v0,0x1
800495c4:  sw ra,0x28(sp)
800495c8:  sw s1,0x24(sp)
800495cc:  sw s0,0x20(sp)
800495d0:  sb v0,0x0(v1)
800495d4:  lui a0,0x8006
800495d8:  lw a0,0x350(a0)
800495dc:  nop
800495e0:  lbu v0,0x0(a0)
800495e4:  nop
800495e8:  andi v0,v0,0x7
800495ec:  sb v0,0x10(sp)
800495f0:  lbu v0,0x10(sp)
800495f4:  nop
800495f8:  beq v0,zero,0x80049af8
800495fc:  _clear s1
80049600:  j 0x80049618
80049604:  _nop
80049608:  lbu v0,0x0(a0)
8004960c:  nop
80049610:  andi v0,v0,0x7
80049614:  sb v0,0x10(sp)
