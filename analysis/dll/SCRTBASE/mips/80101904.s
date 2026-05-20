# addr: 0x80101904  name: FUN_80101904
80101904:  addiu sp,sp,-0x20
80101908:  sw s0,0x10(sp)
8010190c:  move s0,a0
80101910:  sw s2,0x18(sp)
80101914:  move s2,a2
80101918:  li a0,0x2
8010191c:  sw ra,0x1c(sp)
80101920:  beq a1,a0,0x80101a38
80101924:  _sw s1,0x14(sp)
80101928:  sltiu v0,a1,0x3
8010192c:  beq v0,zero,0x80101944
80101930:  _li v0,0x1
80101934:  beq a1,v0,0x80101a68
80101938:  _nop
8010193c:  slti a0,zero,0x6ac
80101940:  nop
80101944:  li v0,0x3
80101948:  beq a1,v0,0x80101960
8010194c:  _li v0,0x8
80101950:  beq a1,v0,0x80101a18
80101954:  _move a0,s0
80101958:  slti a0,zero,0x6ac
8010195c:  nop
80101960:  lw v1,0xc(s2)
80101964:  nop
80101968:  lbu v0,0x4(v1)
8010196c:  nop
80101970:  bne v0,a1,0x80101a04
80101974:  _nop
80101978:  lw v0,0x0(s2)
8010197c:  nop
80101980:  lbu s1,0x4(v0)
80101984:  nop
80101988:  bne s1,a0,0x80101a08
8010198c:  _move a0,s0
80101990:  lw v0,0x0(v1)
80101994:  nop
80101998:  ori v0,v0,0x20
8010199c:  sw v0,0x0(v1)
801019a0:  lb v1,0x8(s0)
801019a4:  li v0,0x1
801019a8:  bne v1,v0,0x80101ab0
801019ac:  _nop
801019b0:  lw a1,0x0(s2)
801019b4:  sltiu a0,zero,0x60f
801019b8:  move a0,s0
801019bc:  lhu v1,0x46(s0)
801019c0:  lw v0,0x60(s0)
801019c4:  sb s1,0x8(s0)
801019c8:  lhu a1,0x0(v0)
801019cc:  move a0,s0
801019d0:  jal 0x8001f9cc
801019d4:  _addu a1,v1,a1
801019d8:  jal 0x8004410c
801019dc:  _nop
801019e0:  lw a3,0x0(s2)
801019e4:  lui v1,0x8006
801019e8:  lw a1,0x58fc(v1)
801019ec:  move a0,v0
801019f0:  li a2,0x14
801019f4:  jal 0x800447e8
801019f8:  _addiu a3,a3,0x24
801019fc:  slti a0,zero,0x6ac
80101a00:  nop
80101a04:  move a0,s0
80101a08:  jal 0x8002239c
80101a0c:  _move a1,s2
80101a10:  slti a0,zero,0x688
80101a14:  nop
80101a18:  jal 0x80022320
80101a1c:  _move a1,s2
80101a20:  beq v0,zero,0x80101ab0
80101a24:  _nop
80101a28:  jal 0x80020844
80101a2c:  _move a0,s0
80101a30:  slti a0,zero,0x6ac
80101a34:  nop
80101a38:  lw v0,0x78(s0)
80101a3c:  nop
80101a40:  lw v1,0x0(v0)
80101a44:  move a0,s0
80101a48:  li a1,-0x21
80101a4c:  and v1,v1,a1
80101a50:  sw v1,0x0(v0)
80101a54:  slti a0,zero,0x6a7
80101a58:  sb zero,0x8(s0)
80101a5c:  sw v1,0x78(s0)
80101a60:  slti a0,zero,0x6a6
80101a64:  sb v0,0x4(v1)
80101a68:  lw v1,0x38(s0)
80101a6c:  nop
80101a70:  beq v1,zero,0x80101a9c
80101a74:  _move a0,s0
80101a78:  lh v0,0x6(v1)
80101a7c:  nop
80101a80:  beq v0,zero,0x80101a5c
80101a84:  _li v0,0x3
80101a88:  lw v1,0x34(v1)
80101a8c:  nop
80101a90:  bne v1,zero,0x80101a78
80101a94:  _nop
80101a98:  move a0,s0
80101a9c:  jal 0x8001abd0
80101aa0:  _nop
80101aa4:  move a0,s0
80101aa8:  jal 0x8001f9cc
80101aac:  _clear a1
80101ab0:  lw ra,0x1c(sp)
80101ab4:  lw s2,0x18(sp)
80101ab8:  lw s1,0x14(sp)
80101abc:  lw s0,0x10(sp)
80101ac0:  clear v0
80101ac4:  jr ra
80101ac8:  _addiu sp,sp,0x20
