# addr: 0x8004a6fc  name: CD_init
8004a6fc:  addiu sp,sp,-0x18
8004a700:  lui a0,0x8001
8004a704:  addiu a0,a0,0x10b0
8004a708:  sw ra,0x10(sp)
8004a70c:  jal 0x80053884
8004a710:  _nop
8004a714:  lui a0,0x8001
8004a718:  addiu a0,a0,0x10bc
8004a71c:  lui a1,0x8006
8004a720:  jal 0x80052604
8004a724:  _addiu a1,a1,0x360
8004a728:  lui at,0x8006
8004a72c:  sb zero,0x9d(at)
8004a730:  lui at,0x8006
8004a734:  sb zero,0x9c(at)
8004a738:  lui at,0x8006
8004a73c:  sw zero,0x80(at)
8004a740:  lui at,0x8006
8004a744:  sw zero,0x7c(at)
8004a748:  lui at,0x8006
8004a74c:  sw zero,0x90(at)
8004a750:  lui at,0x8006
8004a754:  jal 0x80048054
8004a758:  _sw zero,0x8c(at)
8004a75c:  lui a1,0x8005
8004a760:  addiu a1,a1,-0x53c4
8004a764:  jal 0x80048084
8004a768:  _li a0,0x2
8004a76c:  lui v1,0x8006
8004a770:  lw v1,0x344(v1)
8004a774:  li v0,0x1
8004a778:  sb v0,0x0(v1)
8004a77c:  lui v0,0x8006
8004a780:  lw v0,0x350(v0)
8004a784:  nop
8004a788:  lbu v0,0x0(v0)
8004a78c:  nop
8004a790:  andi v0,v0,0x7
8004a794:  beq v0,zero,0x8004a7f0
8004a798:  _li a0,0x1
8004a79c:  li v1,0x7
8004a7a0:  lui v0,0x8006
8004a7a4:  lw v0,0x344(v0)
8004a7a8:  nop
8004a7ac:  sb a0,0x0(v0)
8004a7b0:  lui v0,0x8006
8004a7b4:  lw v0,0x350(v0)
8004a7b8:  nop
8004a7bc:  sb v1,0x0(v0)
8004a7c0:  lui v0,0x8006
8004a7c4:  lw v0,0x34c(v0)
8004a7c8:  nop
8004a7cc:  sb v1,0x0(v0)
8004a7d0:  lui v0,0x8006
8004a7d4:  lw v0,0x350(v0)
8004a7d8:  nop
8004a7dc:  lbu v0,0x0(v0)
8004a7e0:  nop
8004a7e4:  andi v0,v0,0x7
8004a7e8:  bne v0,zero,0x8004a7a0
8004a7ec:  _nop
8004a7f0:  li a0,0x1
8004a7f4:  clear a1
8004a7f8:  lui v1,0x8006
8004a7fc:  addiu v1,v1,0x35c
8004a800:  sb zero,0x2(v1)
8004a804:  lbu v0,0x2(v1)
8004a808:  clear a2
8004a80c:  sb v0,0x1(v1)
8004a810:  lui a3,0x8006
8004a814:  lw a3,0x344(a3)
8004a818:  li v0,0x2
8004a81c:  sb v0,0x0(v1)
8004a820:  sb zero,0x0(a3)
8004a824:  lui v0,0x8006
8004a828:  lw v0,0x350(v0)
8004a82c:  clear a3
8004a830:  sb zero,0x0(v0)
8004a834:  lui v1,0x8006
8004a838:  lw v1,0x354(v1)
8004a83c:  li v0,0x1325
8004a840:  jal 0x8004a058
8004a844:  _sw v0,0x0(v1)
8004a848:  lui v0,0x8006
8004a84c:  lw v0,0x8c(v0)
8004a850:  nop
8004a854:  andi v0,v0,0x10
8004a858:  beq v0,zero,0x8004a870
8004a85c:  _li a0,0x1
8004a860:  clear a1
8004a864:  clear a2
8004a868:  jal 0x8004a058
8004a86c:  _clear a3
8004a870:  li a0,0xa
8004a874:  clear a1
8004a878:  clear a2
8004a87c:  jal 0x8004a058
8004a880:  _clear a3
8004a884:  bne v0,zero,0x8004a8cc
8004a888:  _li v0,-0x1
8004a88c:  li a0,0xc
8004a890:  clear a1
8004a894:  clear a2
8004a898:  jal 0x8004a058
8004a89c:  _clear a3
8004a8a0:  bne v0,zero,0x8004a8c8
8004a8a4:  _clear a0
8004a8a8:  jal 0x80049b10
8004a8ac:  _clear a1
8004a8b0:  move a0,v0
8004a8b4:  li v1,0x2
8004a8b8:  bne a0,v1,0x8004a8cc
8004a8bc:  _li v0,-0x1
8004a8c0:  j 0x8004a8cc
8004a8c4:  _clear v0
8004a8c8:  li v0,-0x1
