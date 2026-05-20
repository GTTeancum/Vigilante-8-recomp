# addr: 0x8001f5a0  name: FUN_8001f5a0
8001f5a0:  addiu sp,sp,-0x50
8001f5a4:  sw s8,0x48(sp)
8001f5a8:  move s8,a0
8001f5ac:  sw s5,0x3c(sp)
8001f5b0:  move s5,a1
8001f5b4:  sw ra,0x4c(sp)
8001f5b8:  sw s7,0x44(sp)
8001f5bc:  sw s6,0x40(sp)
8001f5c0:  sw s4,0x38(sp)
8001f5c4:  sw s3,0x34(sp)
8001f5c8:  sw s2,0x30(sp)
8001f5cc:  sw s1,0x2c(sp)
8001f5d0:  sw s0,0x28(sp)
8001f5d4:  lw a0,0x10(s5)
8001f5d8:  jal 0x8001d624
8001f5dc:  _nop
8001f5e0:  lw a0,0x4(s5)
8001f5e4:  nop
8001f5e8:  lhu v1,0x0(a0)
8001f5ec:  move s6,v0
8001f5f0:  li v0,0x1
8001f5f4:  bne v1,v0,0x8001f940
8001f5f8:  _nop
8001f5fc:  lw a1,0x8(s5)
8001f600:  nop
8001f604:  lhu a2,0x0(a1)
8001f608:  nop
8001f60c:  beq a2,v1,0x8001f628
8001f610:  _addiu s3,a0,0x4
8001f614:  li v0,0x2
8001f618:  beq a2,v0,0x8001f82c
8001f61c:  _lui s4,0x8000
8001f620:  j 0x8001f940
8001f624:  _nop
8001f628:  clear s2
8001f62c:  lui s7,0x8000
8001f630:  move s0,s2
8001f634:  addiu s1,sp,0x18
8001f638:  addiu s4,a1,0x4
8001f63c:  xori v0,s0,0x3
8001f640:  sltiu v0,v0,0x1
8001f644:  bne s0,zero,0x8001f658
8001f648:  _move v1,v0
8001f64c:  addiu v0,v0,-0x1
8001f650:  j 0x8001f65c
8001f654:  _sll v0,v0,0xc
8001f658:  sll v0,v1,0xc
8001f65c:  sh v0,0x18(sp)
8001f660:  lhu v0,0x0(s1)
8001f664:  xori v0,s0,0x4
8001f668:  sltiu v1,v0,0x1
8001f66c:  li v0,0x1
8001f670:  bne s0,v0,0x8001f684
8001f674:  _move a0,v1
8001f678:  addiu v0,v1,-0x1
8001f67c:  j 0x8001f688
8001f680:  _sll v0,v0,0xc
8001f684:  sll v0,a0,0xc
8001f688:  sh v0,0x2(s1)
8001f68c:  lhu v0,0x2(s1)
8001f690:  xori v0,s0,0x5
8001f694:  sltiu v1,v0,0x1
8001f698:  li v0,0x2
8001f69c:  bne s0,v0,0x8001f6b0
8001f6a0:  _move a0,v1
8001f6a4:  addiu v0,v1,-0x1
8001f6a8:  j 0x8001f6b4
8001f6ac:  _sll v0,v0,0xc
8001f6b0:  sll v0,a0,0xc
8001f6b4:  sh v0,0x4(s1)
8001f6b8:  lhu v0,0x4(s1)
8001f6bc:  lw v1,0x0(s4)
8001f6c0:  slti v0,s0,0x3
8001f6c4:  beq v0,zero,0x8001f6d4
8001f6c8:  _sw v1,0x20(sp)
8001f6cc:  subu v0,zero,v1
8001f6d0:  sw v0,0x20(sp)
8001f6d4:  move a0,s3
8001f6d8:  addiu a1,s8,0x10
8001f6dc:  addiu a2,sp,0x18
8001f6e0:  jal 0x8001e6dc
8001f6e4:  _move a3,s6
8001f6e8:  move v1,v0
8001f6ec:  slt v0,s7,v1
8001f6f0:  beq v0,zero,0x8001f700
8001f6f4:  _nop
8001f6f8:  move s7,v1
8001f6fc:  move s2,s0
8001f700:  addiu s0,s0,0x1
8001f704:  slti v0,s0,0x6
8001f708:  bne v0,zero,0x8001f63c
8001f70c:  _addiu s4,s4,0x4
8001f710:  addiu v1,sp,0x10
8001f714:  xori v0,s2,0x3
8001f718:  bne s2,zero,0x8001f724
8001f71c:  _sltiu v0,v0,0x1
8001f720:  addiu v0,v0,-0x1
8001f724:  sll v0,v0,0xc
8001f728:  sh v0,0x10(sp)
8001f72c:  lhu v0,0x0(v1)
8001f730:  xori v0,s2,0x4
8001f734:  sltiu a0,v0,0x1
8001f738:  li v0,0x1
8001f73c:  bne s2,v0,0x8001f750
8001f740:  _move a1,a0
8001f744:  addiu v0,a0,-0x1
8001f748:  j 0x8001f754
8001f74c:  _sll v0,v0,0xc
8001f750:  sll v0,a1,0xc
8001f754:  sh v0,0x2(v1)
8001f758:  lhu v0,0x2(v1)
8001f75c:  xori v0,s2,0x5
8001f760:  sltiu a0,v0,0x1
8001f764:  li v0,0x2
8001f768:  bne s2,v0,0x8001f77c
8001f76c:  _move a1,a0
8001f770:  addiu v0,a0,-0x1
8001f774:  j 0x8001f780
8001f778:  _sll v0,v0,0xc
8001f77c:  sll v0,a1,0xc
8001f780:  sh v0,0x4(v1)
8001f784:  move a0,s6
8001f788:  addiu a1,sp,0x10
8001f78c:  addiu s0,s5,0x20
8001f790:  lhu v0,0x4(v1)
8001f794:  jal 0x8004d2b4
8001f798:  _move a2,s0
8001f79c:  addiu a0,s8,0x10
8001f7a0:  move a1,s0
8001f7a4:  jal 0x800434f8
8001f7a8:  _addiu a2,s5,0x28
8001f7ac:  lh v0,0x28(s5)
8001f7b0:  nop
8001f7b4:  bgez v0,0x8001f7c8
8001f7b8:  _addiu v1,s5,0x14
8001f7bc:  lw v0,0xc(s3)
8001f7c0:  j 0x8001f7d4
8001f7c4:  _sw v0,0x0(v1)
8001f7c8:  lw v0,0x0(s3)
8001f7cc:  nop
8001f7d0:  sw v0,0x0(v1)
8001f7d4:  lh v0,0x2a(s5)
8001f7d8:  nop
8001f7dc:  bgez v0,0x8001f7f0
8001f7e0:  _nop
8001f7e4:  lw v0,0x10(s3)
8001f7e8:  j 0x8001f7fc
8001f7ec:  _sw v0,0x4(v1)
8001f7f0:  lw v0,0x4(s3)
8001f7f4:  nop
8001f7f8:  sw v0,0x4(v1)
8001f7fc:  lh v0,0x2c(s5)
8001f800:  nop
8001f804:  bgez v0,0x8001f818
8001f808:  _nop
8001f80c:  lw a0,0x14(s3)
8001f810:  j 0x8001f824
8001f814:  _sw a0,0x8(v1)
8001f818:  lw a0,0x8(s3)
8001f81c:  nop
8001f820:  sw a0,0x8(v1)
8001f824:  j 0x8001f940
8001f828:  _sw s7,0x30(s5)
8001f82c:  lhu v0,0x2(a1)
8001f830:  clear s0
8001f834:  beq v0,zero,0x8001f88c
8001f838:  _move s2,s0
8001f83c:  li s1,0x4
8001f840:  lw a2,0x8(s5)
8001f844:  move a0,s3
8001f848:  addiu a1,s8,0x10
8001f84c:  move a3,s6
8001f850:  jal 0x8001e6dc
8001f854:  _addu a2,a2,s1
8001f858:  move v1,v0
8001f85c:  slt v0,s4,v1
8001f860:  beq v0,zero,0x8001f870
8001f864:  _nop
8001f868:  move s4,v1
8001f86c:  move s2,s0
8001f870:  lw v0,0x8(s5)
8001f874:  nop
8001f878:  lhu v0,0x2(v0)
8001f87c:  addiu s0,s0,0x1
8001f880:  slt v0,s0,v0
8001f884:  bne v0,zero,0x8001f840
8001f888:  _addiu s1,s1,0xc
8001f88c:  move a0,s6
8001f890:  addiu s0,s5,0x20
8001f894:  lw v0,0x8(s5)
8001f898:  move a2,s0
8001f89c:  sll a1,s2,0x1
8001f8a0:  addu a1,a1,s2
8001f8a4:  sll a1,a1,0x2
8001f8a8:  addiu a1,a1,0x4
8001f8ac:  jal 0x8004d2b4
8001f8b0:  _addu a1,v0,a1
8001f8b4:  addiu a0,s8,0x10
8001f8b8:  move a1,s0
8001f8bc:  jal 0x800434f8
8001f8c0:  _addiu a2,s5,0x28
8001f8c4:  lh v0,0x28(s5)
8001f8c8:  nop
8001f8cc:  bgez v0,0x8001f8e0
8001f8d0:  _addiu v1,s5,0x14
8001f8d4:  lw v0,0xc(s3)
8001f8d8:  j 0x8001f8ec
8001f8dc:  _sw v0,0x0(v1)
8001f8e0:  lw v0,0x0(s3)
8001f8e4:  nop
8001f8e8:  sw v0,0x0(v1)
8001f8ec:  lh v0,0x2a(s5)
8001f8f0:  nop
8001f8f4:  bgez v0,0x8001f908
8001f8f8:  _nop
8001f8fc:  lw v0,0x10(s3)
8001f900:  j 0x8001f914
8001f904:  _sw v0,0x4(v1)
8001f908:  lw v0,0x4(s3)
8001f90c:  nop
8001f910:  sw v0,0x4(v1)
8001f914:  lh v0,0x2c(s5)
8001f918:  nop
8001f91c:  bgez v0,0x8001f930
8001f920:  _nop
8001f924:  lw a0,0x14(s3)
8001f928:  j 0x8001f93c
8001f92c:  _sw a0,0x8(v1)
8001f930:  lw a0,0x8(s3)
8001f934:  nop
8001f938:  sw a0,0x8(v1)
8001f93c:  sw s4,0x30(s5)
8001f940:  lw ra,0x4c(sp)
8001f944:  lw s8,0x48(sp)
8001f948:  lw s7,0x44(sp)
8001f94c:  lw s6,0x40(sp)
8001f950:  move v0,s5
8001f954:  lw s5,0x3c(sp)
8001f958:  lw s4,0x38(sp)
8001f95c:  lw s3,0x34(sp)
8001f960:  lw s2,0x30(sp)
8001f964:  lw s1,0x2c(sp)
8001f968:  lw s0,0x28(sp)
8001f96c:  jr ra
8001f970:  _addiu sp,sp,0x50
