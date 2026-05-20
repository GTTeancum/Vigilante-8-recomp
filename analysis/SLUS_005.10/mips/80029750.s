# addr: 0x80029750  name: FUN_80029750
80029750:  lw a1,0x4(gp)
80029754:  addiu sp,sp,-0x68
80029758:  sw s0,0x50(sp)
8002975c:  addiu s0,sp,0x10
80029760:  move a0,s0
80029764:  lui v0,0x8007
80029768:  sw ra,0x64(sp)
8002976c:  sw s4,0x60(sp)
80029770:  sw s3,0x5c(sp)
80029774:  sw s2,0x58(sp)
80029778:  sw s1,0x54(sp)
8002977c:  addiu t5,v0,-0x8c0
80029780:  lw t2,0x0(t5)
80029784:  lw t3,0x4(t5)
80029788:  lw t4,0x8(t5)
8002978c:  sw t2,0x10(sp)
80029790:  sw t3,0x14(sp)
80029794:  sw t4,0x18(sp)
80029798:  lw t2,0xc(t5)
8002979c:  lw t3,0x10(t5)
800297a0:  lw t4,0x14(t5)
800297a4:  sw t2,0x1c(sp)
800297a8:  sw t3,0x20(sp)
800297ac:  sw t4,0x24(sp)
800297b0:  lw t2,0x18(t5)
800297b4:  lw t3,0x1c(t5)
800297b8:  sw t2,0x28(sp)
800297bc:  sw t3,0x2c(sp)
800297c0:  lui v0,0x8009
800297c4:  addiu v0,v0,0x10c0
800297c8:  sll v1,a1,0x1
800297cc:  addu v1,v1,a1
800297d0:  sll v1,v1,0x4
800297d4:  addu s1,v1,v0
800297d8:  sll v1,a1,0x2
800297dc:  addu v1,v1,a1
800297e0:  sll v1,v1,0x4
800297e4:  lui v0,0x8009
800297e8:  addiu v0,v0,0x1020
800297ec:  jal 0x80016c88
800297f0:  _addu s2,v1,v0
800297f4:  move s4,v0
800297f8:  sll a0,s4,0x10
800297fc:  sra a0,a0,0x10
80029800:  subu a0,zero,a0
80029804:  jal 0x8004d914
80029808:  _move a1,s0
8002980c:  jal 0x80016c54
80029810:  _move a0,s0
80029814:  move v1,v0
80029818:  sll v0,v1,0x10
8002981c:  sra v0,v0,0x10
80029820:  bgez v0,0x8002982c
80029824:  _nop
80029828:  subu v0,zero,v0
8002982c:  slti v0,v0,0x401
80029830:  bne v0,zero,0x80029840
80029834:  _nop
80029838:  li v0,0x800
8002983c:  subu v1,v0,v1
80029840:  sll v0,v1,0x10
80029844:  sra v0,v0,0x10
80029848:  subu s3,zero,v0
8002984c:  move a0,s3
80029850:  jal 0x8004d774
80029854:  _move a1,s0
80029858:  andi v1,s4,0xfff
8002985c:  sll v0,v1,0x2
80029860:  addu v1,v0,v1
80029864:  bgez v1,0x80029870
80029868:  _move v0,v1
8002986c:  addiu v0,v1,0xfff
80029870:  move a0,s0
80029874:  addiu s0,sp,0x30
80029878:  move a1,s0
8002987c:  andi v0,v0,0xf000
80029880:  subu v0,v1,v0
80029884:  sw v0,0x24(sp)
80029888:  sll v0,s3,0x2
8002988c:  addu v0,v0,s3
80029890:  sw v0,0x28(sp)
80029894:  li v0,-0xd00
80029898:  jal 0x80016dfc
8002989c:  _sw v0,0x2c(sp)
800298a0:  jal 0x80043974
800298a4:  _move a0,s0
800298a8:  lui v1,0x8006
800298ac:  addiu v1,v1,-0x164c
800298b0:  ldv3c v1
800298c8:  nRTPT
800298d4:  lw a0,0x60c(gp)
800298d8:  nop
800298dc:  lw a1,0x3ffc(a0)
800298e0:  sll v0,s2,0x8
800298e4:  srl v0,v0,0x8
800298e8:  sw v0,0x3ffc(a0)
800298ec:  lbu v0,0x3(s2)
800298f0:  nop
800298f4:  sll v0,v0,0x18
800298f8:  or v0,v0,a1
800298fc:  sw v0,0x0(s2)
80029900:  gte_stSXY0 0x8(s2)
80029904:  gte_stSXY1 0x10(s2)
80029908:  gte_stSXY2 0x38(s2)
8002990c:  gte_stSXY1 0x30(s2)
80029910:  gte_ldVXY0 0x18(v1)
80029914:  gte_ldVZ0 0x1c(v1)
80029918:  gte_ldVXY1 0x20(v1)
8002991c:  gte_ldVZ1 0x24(v1)
80029920:  gte_ldVXY2 0x28(v1)
80029924:  gte_ldVZ2 0x2c(v1)
80029928:  nRTPT
80029934:  lw v1,0x60c(gp)
80029938:  nop
8002993c:  lw a0,0x3ffc(v1)
80029940:  addiu v0,s2,0x28
80029944:  sll v0,v0,0x8
80029948:  srl v0,v0,0x8
8002994c:  sw v0,0x3ffc(v1)
80029950:  lbu v0,0x2b(s2)
80029954:  nop
80029958:  sll v0,v0,0x18
8002995c:  or v0,v0,a0
80029960:  sw v0,0x28(s2)
80029964:  gte_stSXY0 0x18(s2)
80029968:  gte_stSXY1 0x20(s2)
8002996c:  gte_stSXY2 0x48(s2)
80029970:  gte_stSXY1 0x40(s2)
80029974:  lh v1,0x10(s2)
80029978:  lh v0,0x20(s2)
8002997c:  lh a0,0x12(s2)
80029980:  lh a1,0x22(s2)
80029984:  lh a3,0x30(sp)
80029988:  addu v1,v1,v0
8002998c:  srl v0,v1,0x1f
80029990:  addu v1,v1,v0
80029994:  addu a0,a0,a1
80029998:  srl v0,a0,0x1f
8002999c:  addu a0,a0,v0
800299a0:  sra t0,a0,0x1
800299a4:  slti v0,a3,0xb51
800299a8:  lh a0,0x36(sp)
800299ac:  bne v0,zero,0x80029a4c
800299b0:  _sra t1,v1,0x1
800299b4:  subu v0,zero,t1
800299b8:  mult v0,a0
800299bc:  mflo v1
800299c0:  nop
800299c4:  nop
800299c8:  div v1,a3
800299cc:  mflo a2
800299d0:  lw v0,0x6d8(gp)
800299d4:  nop
800299d8:  subu v0,v0,t1
800299dc:  mult v0,a0
800299e0:  mflo v1
800299e4:  nop
800299e8:  nop
800299ec:  div v1,a3
800299f0:  mflo a1
800299f4:  lhu a0,0x6dc(gp)
800299f8:  lhu v1,0x6d8(gp)
800299fc:  sh zero,0x16(s1)
80029a00:  sh zero,0x12(s1)
80029a04:  addiu v0,s1,0x18
80029a08:  sh a0,0x16(v0)
80029a0c:  sh a0,0x2a(s1)
80029a10:  sh zero,0x10(s1)
80029a14:  sh zero,0x8(s1)
80029a18:  sh zero,0x10(v0)
80029a1c:  sh zero,0x20(s1)
80029a20:  sh v1,0x14(s1)
80029a24:  sh v1,0xc(s1)
80029a28:  sh v1,0x14(v0)
80029a2c:  sh v1,0x24(s1)
80029a30:  addu a2,t0,a2
80029a34:  sh a2,0xa(v0)
80029a38:  sh a2,0xa(s1)
80029a3c:  addu a1,t0,a1
80029a40:  sh a1,0xe(v0)
80029a44:  j 0x80029c18
80029a48:  _sh a1,0xe(s1)
80029a4c:  slti v0,a3,-0xb50
80029a50:  beq v0,zero,0x80029aec
80029a54:  _subu v0,zero,t1
80029a58:  mult v0,a0
80029a5c:  mflo v1
80029a60:  nop
80029a64:  nop
80029a68:  div v1,a3
80029a6c:  mflo a1
80029a70:  lw v0,0x6d8(gp)
80029a74:  nop
80029a78:  subu v0,v0,t1
80029a7c:  mult v0,a0
80029a80:  mflo v1
80029a84:  nop
80029a88:  nop
80029a8c:  div v1,a3
80029a90:  mflo a0
80029a94:  lhu v0,0x6dc(gp)
80029a98:  lhu v1,0x6d8(gp)
80029a9c:  sh v0,0x16(s1)
80029aa0:  sh v0,0x12(s1)
80029aa4:  addiu v0,s1,0x18
80029aa8:  sh zero,0x16(v0)
80029aac:  sh zero,0x2a(s1)
80029ab0:  sh zero,0x10(s1)
80029ab4:  sh zero,0x8(s1)
80029ab8:  sh zero,0x10(v0)
80029abc:  sh zero,0x20(s1)
80029ac0:  sh v1,0x14(s1)
80029ac4:  sh v1,0xc(s1)
80029ac8:  sh v1,0x14(v0)
80029acc:  sh v1,0x24(s1)
80029ad0:  addu a1,t0,a1
80029ad4:  sh a1,0xa(v0)
80029ad8:  sh a1,0xa(s1)
80029adc:  addu a0,t0,a0
80029ae0:  sh a0,0xe(v0)
80029ae4:  j 0x80029c18
80029ae8:  _sh a0,0xe(s1)
80029aec:  blez a0,0x80029b88
80029af0:  _subu v0,zero,t0
80029af4:  mult v0,a3
80029af8:  mflo v1
80029afc:  nop
80029b00:  nop
80029b04:  div v1,a0
80029b08:  mflo a1
80029b0c:  lw v0,0x6dc(gp)
80029b10:  nop
80029b14:  subu v0,v0,t0
80029b18:  mult v0,a3
80029b1c:  mflo v1
80029b20:  nop
80029b24:  nop
80029b28:  div v1,a0
80029b2c:  mflo a0
80029b30:  lhu v0,0x6d8(gp)
80029b34:  lhu v1,0x6dc(gp)
80029b38:  sh v0,0x14(s1)
80029b3c:  sh v0,0x10(s1)
80029b40:  addiu v0,s1,0x18
80029b44:  sh zero,0x14(v0)
80029b48:  sh zero,0x28(s1)
80029b4c:  sh zero,0x12(s1)
80029b50:  sh zero,0xa(s1)
80029b54:  sh zero,0x12(v0)
80029b58:  sh zero,0x22(s1)
80029b5c:  sh v1,0x16(s1)
80029b60:  sh v1,0xe(s1)
80029b64:  sh v1,0x16(v0)
80029b68:  sh v1,0x26(s1)
80029b6c:  addu a1,t1,a1
80029b70:  sh a1,0x8(v0)
80029b74:  sh a1,0x8(s1)
80029b78:  addu a0,t1,a0
80029b7c:  sh a0,0xc(v0)
80029b80:  j 0x80029c18
80029b84:  _sh a0,0xc(s1)
80029b88:  mult v0,a3
80029b8c:  mflo v1
80029b90:  nop
80029b94:  nop
80029b98:  div v1,a0
80029b9c:  mflo a2
80029ba0:  lw v0,0x6dc(gp)
80029ba4:  nop
80029ba8:  subu v0,v0,t0
80029bac:  mult v0,a3
80029bb0:  mflo v1
80029bb4:  nop
80029bb8:  nop
80029bbc:  div v1,a0
80029bc0:  mflo a1
80029bc4:  lhu a0,0x6d8(gp)
80029bc8:  lhu v1,0x6dc(gp)
80029bcc:  sh zero,0x14(s1)
80029bd0:  sh zero,0x10(s1)
80029bd4:  addiu v0,s1,0x18
80029bd8:  sh a0,0x14(v0)
80029bdc:  sh a0,0x28(s1)
80029be0:  sh zero,0x12(s1)
80029be4:  sh zero,0xa(s1)
80029be8:  sh zero,0x12(v0)
80029bec:  sh zero,0x22(s1)
80029bf0:  sh v1,0x16(s1)
80029bf4:  sh v1,0xe(s1)
80029bf8:  sh v1,0x16(v0)
80029bfc:  sh v1,0x26(s1)
80029c00:  addu a2,t1,a2
80029c04:  sh a2,0x8(v0)
80029c08:  sh a2,0x8(s1)
80029c0c:  addu a1,t1,a1
80029c10:  sh a1,0xc(v0)
80029c14:  sh a1,0xc(s1)
80029c18:  lw v1,0x60c(gp)
80029c1c:  nop
80029c20:  lw a0,0x3ffc(v1)
80029c24:  sll v0,s1,0x8
80029c28:  srl v0,v0,0x8
80029c2c:  sw v0,0x3ffc(v1)
80029c30:  lbu v0,0x3(s1)
80029c34:  nop
80029c38:  sll v0,v0,0x18
80029c3c:  or v0,v0,a0
80029c40:  sw v0,0x0(s1)
80029c44:  lw ra,0x64(sp)
80029c48:  lw s4,0x60(sp)
80029c4c:  lw s3,0x5c(sp)
80029c50:  lw s2,0x58(sp)
80029c54:  lw s1,0x54(sp)
80029c58:  lw s0,0x50(sp)
80029c5c:  jr ra
80029c60:  _addiu sp,sp,0x68
