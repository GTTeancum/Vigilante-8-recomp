# addr: 0x8001af48  name: FUN_8001af48
8001af48:  addiu sp,sp,-0x20
8001af4c:  sw s1,0x14(sp)
8001af50:  move s1,a0
8001af54:  sw ra,0x18(sp)
8001af58:  beq s1,zero,0x8001af8c
8001af5c:  _sw s0,0x10(sp)
8001af60:  lw a0,0x30(s1)
8001af64:  jal 0x8001bddc
8001af68:  _move s0,s1
8001af6c:  lw a0,0x38(s1)
8001af70:  jal 0x8001af48
8001af74:  _nop
8001af78:  lw s1,0x34(s1)
8001af7c:  jal 0x80045088
8001af80:  _move a0,s0
8001af84:  bne s1,zero,0x8001af60
8001af88:  _nop
8001af8c:  lw ra,0x18(sp)
8001af90:  lw s1,0x14(sp)
8001af94:  lw s0,0x10(sp)
8001af98:  jr ra
8001af9c:  _addiu sp,sp,0x20
