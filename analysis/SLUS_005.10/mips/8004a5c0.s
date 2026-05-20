# addr: 0x8004a5c0  name: CD_initvol
8004a5c0:  lui v1,0x8006
8004a5c4:  lw v1,0x358(v1)
8004a5c8:  nop
8004a5cc:  lhu v0,0x1b8(v1)
8004a5d0:  nop
8004a5d4:  bne v0,zero,0x8004a5fc
8004a5d8:  _addiu sp,sp,-0x8
8004a5dc:  lhu v0,0x1ba(v1)
8004a5e0:  nop
8004a5e4:  bne v0,zero,0x8004a600
8004a5e8:  _li v0,0x3fff
8004a5ec:  sh v0,0x180(v1)
8004a5f0:  sh v0,0x182(v1)
8004a5f4:  lui v1,0x8006
8004a5f8:  lw v1,0x358(v1)
8004a5fc:  li v0,0x3fff
8004a600:  sh v0,0x1b0(v1)
8004a604:  sh v0,0x1b2(v1)
8004a608:  ori v0,zero,0xc001
8004a60c:  sh v0,0x1aa(v1)
8004a610:  lui v1,0x8006
8004a614:  lw v1,0x344(v1)
8004a618:  li v0,0x80
8004a61c:  sb v0,0x2(sp)
8004a620:  sb v0,0x0(sp)
8004a624:  li v0,0x2
8004a628:  sb zero,0x3(sp)
8004a62c:  sb zero,0x1(sp)
8004a630:  sb v0,0x0(v1)
8004a634:  lui v1,0x8006
8004a638:  lw v1,0x34c(v1)
8004a63c:  lbu v0,0x0(sp)
8004a640:  nop
8004a644:  sb v0,0x0(v1)
8004a648:  lui v1,0x8006
8004a64c:  lw v1,0x350(v1)
8004a650:  lbu v0,0x1(sp)
8004a654:  nop
8004a658:  sb v0,0x0(v1)
8004a65c:  lui v1,0x8006
8004a660:  lw v1,0x344(v1)
8004a664:  li v0,0x3
8004a668:  sb v0,0x0(v1)
8004a66c:  lui v1,0x8006
8004a670:  lw v1,0x348(v1)
8004a674:  lbu v0,0x2(sp)
8004a678:  nop
8004a67c:  sb v0,0x0(v1)
8004a680:  lui v1,0x8006
8004a684:  lw v1,0x34c(v1)
8004a688:  lbu v0,0x3(sp)
8004a68c:  nop
8004a690:  sb v0,0x0(v1)
8004a694:  lui v1,0x8006
8004a698:  lw v1,0x350(v1)
8004a69c:  li v0,0x20
8004a6a0:  sb v0,0x0(v1)
8004a6a4:  clear v0
8004a6a8:  jr ra
8004a6ac:  _addiu sp,sp,0x8
