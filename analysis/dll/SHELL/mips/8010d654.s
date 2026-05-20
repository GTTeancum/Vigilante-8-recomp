# addr: 0x8010d654  name: FUN_8010d654
8010d654:  addiu sp,sp,-0x28
8010d658:  sw s1,0x1c(sp)
8010d65c:  move s1,a0
8010d660:  sw s0,0x18(sp)
8010d664:  lui s0,0x3
8010d668:  ori s0,s0,0xd40
8010d66c:  sw ra,0x20(sp)
8010d670:  addiu a0,sp,0x10
8010d674:  sltiu a0,zero,0x3788
8010d678:  addiu a1,sp,0x14
8010d67c:  beq v0,zero,0x8010d694
8010d680:  _addiu s0,s0,-0x1
8010d684:  bne s0,zero,0x8010d674
8010d688:  _addiu a0,sp,0x10
8010d68c:  slti a0,zero,0x35d0
8010d690:  li v0,-0x1
8010d694:  lw a0,0x14(sp)
8010d698:  lw v1,0x48(s1)
8010d69c:  lw v0,0x8(a0)
8010d6a0:  nop
8010d6a4:  sltu v0,v0,v1
8010d6a8:  bne v0,zero,0x8010d740
8010d6ac:  _clear v0
8010d6b0:  lw v0,0x4c(s1)
8010d6b4:  nop
8010d6b8:  beq v0,zero,0x8010d6d8
8010d6bc:  _nop
8010d6c0:  lhu v1,0x10(a0)
8010d6c4:  nop
8010d6c8:  sll v0,v1,0x1
8010d6cc:  addu v0,v0,v1
8010d6d0:  slti a0,zero,0x35b7
8010d6d4:  srl v0,v0,0x1
8010d6d8:  lhu v0,0x10(a0)
8010d6dc:  lw a0,0x10(sp)
8010d6e0:  lw v1,0x14(sp)
8010d6e4:  lw a1,0x18(s1)
8010d6e8:  lw a2,0x0(s1)
8010d6ec:  sh v0,0x34(s1)
8010d6f0:  sh v0,0x2c(s1)
8010d6f4:  lhu v0,0x12(v1)
8010d6f8:  nop
8010d6fc:  sh v0,0x42(s1)
8010d700:  sh v0,0x36(s1)
8010d704:  sh v0,0x2e(s1)
8010d708:  lw v1,0x8(v1)
8010d70c:  li v0,0x1
8010d710:  subu v0,v0,a1
8010d714:  sw v0,0x18(s1)
8010d718:  sll v0,v0,0x2
8010d71c:  addu v0,s1,v0
8010d720:  sw v1,0x48(s1)
8010d724:  lw a1,0x8(v0)
8010d728:  sltiu a0,zero,0x3a04
8010d72c:  nop
8010d730:  lw a0,0x10(sp)
8010d734:  sltiu a0,zero,0x375c
8010d738:  nop
8010d73c:  lw v0,0x48(s1)
8010d740:  lw ra,0x20(sp)
8010d744:  lw s1,0x1c(sp)
8010d748:  lw s0,0x18(sp)
8010d74c:  jr ra
8010d750:  _addiu sp,sp,0x28
