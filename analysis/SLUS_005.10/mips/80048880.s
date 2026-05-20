# addr: 0x80048880  name: INTR_DMA_OBJ_4C
80048880:  lui v0,0x8006
80048884:  lw v0,-0x3c(v0)
80048888:  addiu sp,sp,-0x30
8004888c:  sw ra,0x28(sp)
80048890:  sw s5,0x24(sp)
80048894:  sw s4,0x20(sp)
80048898:  sw s3,0x1c(sp)
8004889c:  sw s2,0x18(sp)
800488a0:  sw s1,0x14(sp)
800488a4:  sw s0,0x10(sp)
800488a8:  lw v0,0x0(v0)
800488ac:  nop
800488b0:  srl v0,v0,0x18
800488b4:  andi s1,v0,0x7f
800488b8:  beq s1,zero,0x8004895c
800488bc:  _nop
800488c0:  li s4,0x1
800488c4:  lui s3,0xff
800488c8:  ori s3,s3,0xffff
800488cc:  lui s5,0x8006
800488d0:  addiu s5,s5,-0x38
800488d4:  beq s1,zero,0x80048938
800488d8:  _clear s0
800488dc:  move s2,s5
800488e0:  slti v0,s0,0x7
800488e4:  beq v0,zero,0x80048938
800488e8:  _andi v0,s1,0x1
800488ec:  beq v0,zero,0x80048928
800488f0:  _addiu v0,s0,0x18
800488f4:  lui a0,0x8006
800488f8:  lw a0,-0x3c(a0)
800488fc:  sllv v0,s4,v0
80048900:  lw v1,0x0(a0)
80048904:  or v0,v0,s3
80048908:  and v1,v1,v0
8004890c:  sw v1,0x0(a0)
80048910:  lw v0,0x0(s2)
80048914:  nop
80048918:  beq v0,zero,0x80048928
8004891c:  _nop
80048920:  jalr v0
80048924:  _nop
80048928:  addiu s2,s2,0x4
8004892c:  srl s1,s1,0x1
80048930:  bne s1,zero,0x800488e0
80048934:  _addiu s0,s0,0x1
80048938:  lui v0,0x8006
8004893c:  lw v0,-0x3c(v0)
80048940:  nop
80048944:  lw v0,0x0(v0)
80048948:  nop
8004894c:  srl v0,v0,0x18
80048950:  andi s1,v0,0x7f
80048954:  bne s1,zero,0x800488d4
80048958:  _nop
8004895c:  lui a1,0x8006
80048960:  lw a1,-0x3c(a1)
80048964:  nop
80048968:  lw v0,0x0(a1)
8004896c:  lui v1,0xff00
80048970:  and v0,v0,v1
80048974:  lui v1,0x8000
80048978:  beq v0,v1,0x80048994
8004897c:  _nop
80048980:  lw v0,0x0(a1)
80048984:  nop
80048988:  andi v0,v0,0x8000
8004898c:  beq v0,zero,0x800489dc
80048990:  _nop
80048994:  lui a0,0x8001
80048998:  addiu a0,a0,0xde4
8004899c:  lw a1,0x0(a1)
800489a0:  jal 0x80052604
800489a4:  _clear s0
800489a8:  lui a0,0x8001
800489ac:  addiu a0,a0,0xe00
800489b0:  move a1,s0
800489b4:  lui v0,0x8006
800489b8:  lw v0,-0x18(v0)
800489bc:  sll v1,s0,0x4
800489c0:  addu v1,v1,v0
800489c4:  lw a2,0x0(v1)
800489c8:  jal 0x80052604
800489cc:  _addiu s0,s0,0x1
800489d0:  slti v0,s0,0x7
800489d4:  bne v0,zero,0x800489a8
800489d8:  _nop
800489dc:  lw ra,0x28(sp)
800489e0:  lw s5,0x24(sp)
800489e4:  lw s4,0x20(sp)
800489e8:  lw s3,0x1c(sp)
800489ec:  lw s2,0x18(sp)
800489f0:  lw s1,0x14(sp)
800489f4:  lw s0,0x10(sp)
800489f8:  jr ra
800489fc:  _addiu sp,sp,0x30
