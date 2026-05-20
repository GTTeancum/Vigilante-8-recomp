# addr: 0x80020658  name: FUN_80020658
80020658:  addiu sp,sp,-0x20
8002065c:  sw s1,0x14(sp)
80020660:  move s1,a0
80020664:  sw ra,0x1c(sp)
80020668:  sw s2,0x18(sp)
8002066c:  sw s0,0x10(sp)
80020670:  lw v0,0x8(s1)
80020674:  nop
80020678:  beq v0,s1,0x800206d8
8002067c:  _nop
80020680:  lui s2,0x8006
80020684:  addiu s2,s2,0x5a74
80020688:  lw s0,0x0(s1)
8002068c:  nop
80020690:  lw a0,0x8(s0)
80020694:  jal 0x80020540
80020698:  _nop
8002069c:  lw v1,0x4(s0)
800206a0:  lw v0,0x0(s0)
800206a4:  nop
800206a8:  sw v1,0x4(v0)
800206ac:  sw v0,0x0(v1)
800206b0:  lw v0,0x774(gp)
800206b4:  sw s0,0x774(gp)
800206b8:  sw s0,0x0(v0)
800206bc:  sw v0,0x4(s0)
800206c0:  sw s2,0x0(s0)
800206c4:  sw zero,0x8(s0)
800206c8:  lw v0,0x8(s1)
800206cc:  nop
800206d0:  bne v0,s1,0x80020688
800206d4:  _nop
800206d8:  lw ra,0x1c(sp)
800206dc:  lw s2,0x18(sp)
800206e0:  lw s1,0x14(sp)
800206e4:  lw s0,0x10(sp)
800206e8:  jr ra
800206ec:  _addiu sp,sp,0x20
