# addr: 0x8002accc  name: FUN_8002accc
8002accc:  lw a0,0x8b0(gp)
8002acd0:  addiu sp,sp,-0x20
8002acd4:  sw s1,0x14(sp)
8002acd8:  sw s0,0x10(sp)
8002acdc:  lui s0,0x8006
8002ace0:  addiu s0,s0,0x5b70
8002ace4:  sw ra,0x18(sp)
8002ace8:  jal 0x8001bddc
8002acec:  _clear s1
8002acf0:  sw zero,0x8b0(gp)
8002acf4:  lw a0,0x0(s0)
8002acf8:  addiu s0,s0,0x4
8002acfc:  jal 0x8001bddc
8002ad00:  _addiu s1,s1,0x1
8002ad04:  sltiu v0,s1,0x4
8002ad08:  bne v0,zero,0x8002acf4
8002ad0c:  _nop
8002ad10:  lw a0,0x87c(gp)
8002ad14:  jal 0x800190d8
8002ad18:  _nop
8002ad1c:  lw ra,0x18(sp)
8002ad20:  lw s1,0x14(sp)
8002ad24:  lw s0,0x10(sp)
8002ad28:  jr ra
8002ad2c:  _addiu sp,sp,0x20
