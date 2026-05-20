# addr: 0x8003e598  name: FUN_8003e598
8003e598:  addiu sp,sp,-0x30
8003e59c:  sw s0,0x28(sp)
8003e5a0:  move s0,a0
8003e5a4:  move a0,a1
8003e5a8:  sw ra,0x2c(sp)
8003e5ac:  lw a1,0x5c(s0)
8003e5b0:  nop
8003e5b4:  beq a1,zero,0x8003e720
8003e5b8:  _li a3,0x1
8003e5bc:  lhu v0,0x0(a1)
8003e5c0:  nop
8003e5c4:  beq v0,zero,0x8003e70c
8003e5c8:  _move t1,a3
8003e5cc:  li t0,0x2
8003e5d0:  lhu v0,0x0(a1)
8003e5d4:  nop
8003e5d8:  beq v0,t1,0x8003e5f0
8003e5dc:  _nop
8003e5e0:  beq v0,t0,0x8003e6e0
8003e5e4:  _nop
8003e5e8:  j 0x8003e700
8003e5ec:  _nop
8003e5f0:  beq a3,zero,0x8003e630
8003e5f4:  _move a2,a1
8003e5f8:  lw t2,0x4(a1)
8003e5fc:  lw t3,0x8(a1)
8003e600:  lw t4,0xc(a1)
8003e604:  lw t5,0x10(a1)
8003e608:  sw t2,0x10(sp)
8003e60c:  sw t3,0x14(sp)
8003e610:  sw t4,0x18(sp)
8003e614:  sw t5,0x1c(sp)
8003e618:  lw t2,0x14(a1)
8003e61c:  lw t3,0x18(a1)
8003e620:  sw t2,0x20(sp)
8003e624:  sw t3,0x24(sp)
8003e628:  j 0x8003e6d8
8003e62c:  _clear a3
8003e630:  lw v1,0x4(a1)
8003e634:  lw v0,0x10(sp)
8003e638:  nop
8003e63c:  slt v0,v1,v0
8003e640:  beq v0,zero,0x8003e64c
8003e644:  _nop
8003e648:  sw v1,0x10(sp)
8003e64c:  lw v1,0x8(a1)
8003e650:  lw v0,0x14(sp)
8003e654:  nop
8003e658:  slt v0,v1,v0
8003e65c:  beq v0,zero,0x8003e668
8003e660:  _nop
8003e664:  sw v1,0x14(sp)
8003e668:  lw v1,0xc(a1)
8003e66c:  lw v0,0x18(sp)
8003e670:  nop
8003e674:  slt v0,v1,v0
8003e678:  beq v0,zero,0x8003e684
8003e67c:  _nop
8003e680:  sw v1,0x18(sp)
8003e684:  lw v1,0x10(a1)
8003e688:  lw v0,0x1c(sp)
8003e68c:  nop
8003e690:  slt v0,v0,v1
8003e694:  beq v0,zero,0x8003e6a0
8003e698:  _nop
8003e69c:  sw v1,0x1c(sp)
8003e6a0:  lw v1,0x14(a1)
8003e6a4:  lw v0,0x20(sp)
8003e6a8:  nop
8003e6ac:  slt v0,v0,v1
8003e6b0:  beq v0,zero,0x8003e6bc
8003e6b4:  _nop
8003e6b8:  sw v1,0x20(sp)
8003e6bc:  lw v1,0x18(a1)
8003e6c0:  lw v0,0x24(sp)
8003e6c4:  nop
8003e6c8:  slt v0,v0,v1
8003e6cc:  beq v0,zero,0x8003e6fc
8003e6d0:  _addiu a1,a2,0x1c
8003e6d4:  sw v1,0x24(sp)
8003e6d8:  j 0x8003e6fc
8003e6dc:  _addiu a1,a2,0x1c
8003e6e0:  lhu v1,0x2(a1)
8003e6e4:  nop
8003e6e8:  sll v0,v1,0x1
8003e6ec:  addu v0,v0,v1
8003e6f0:  sll v0,v0,0x2
8003e6f4:  addiu v0,v0,0x4
8003e6f8:  addu a1,a1,v0
8003e6fc:  lhu v0,0x0(a1)
8003e700:  nop
8003e704:  bne v0,zero,0x8003e5d0
8003e708:  _nop
8003e70c:  lw a1,0x1c(sp)
8003e710:  lw a2,0x24(sp)
8003e714:  jal 0x8003e254
8003e718:  _nop
8003e71c:  sw v0,0x70(s0)
8003e720:  lw ra,0x2c(sp)
8003e724:  lw s0,0x28(sp)
8003e728:  jr ra
8003e72c:  _addiu sp,sp,0x30
