# addr: 0x8010167c  name: FUN_8010167c
8010167c:  addiu sp,sp,-0x258
80101680:  sw s3,0x23c(sp)
80101684:  move s3,a0
80101688:  sw s8,0x250(sp)
8010168c:  move s8,a1
80101690:  lui a0,0x8010
80101694:  addiu a0,a0,0x48
80101698:  sw ra,0x254(sp)
8010169c:  sw s7,0x24c(sp)
801016a0:  sw s6,0x248(sp)
801016a4:  sw s5,0x244(sp)
801016a8:  sw s4,0x240(sp)
801016ac:  sw s2,0x238(sp)
801016b0:  sw s1,0x234(sp)
801016b4:  sw s0,0x230(sp)
801016b8:  jal 0x80015f80
801016bc:  _sw a2,0x260(sp)
801016c0:  addiu s2,sp,0xb0
801016c4:  move a0,s2
801016c8:  clear a1
801016cc:  move a2,a1
801016d0:  li a3,0x140
801016d4:  lui v1,0x8006
801016d8:  lui t1,0x8010
801016dc:  sw zero,0x5328(v1)
801016e0:  addiu v1,t1,0x7da0
801016e4:  addiu t0,v1,0x4
801016e8:  sw t0,0x7da0(t1)
801016ec:  lui t1,0x8010
801016f0:  sw zero,0x4(v1)
801016f4:  sw v1,0x8(v1)
801016f8:  addiu v1,t1,0x7d90
801016fc:  addiu t0,v1,0x4
80101700:  li s0,0xf0
80101704:  move s4,v0
80101708:  sw t0,0x7d90(t1)
8010170c:  sw zero,0x4(v1)
80101710:  sw v1,0x8(v1)
80101714:  jal 0x8004f198
80101718:  _sw s0,0x10(sp)
8010171c:  addiu s1,sp,0xc8
80101720:  move a0,s1
80101724:  clear a1
80101728:  move a2,a1
8010172c:  li a3,0x140
80101730:  jal 0x8004f0e4
80101734:  _sw s0,0x10(sp)
80101738:  lui v0,0x8006
8010173c:  lbu v1,0x531c(v0)
80101740:  lui v0,0x8006
80101744:  lbu v0,0x531d(v0)
80101748:  move a0,s1
8010174c:  sll v1,v1,0x18
80101750:  sra v1,v1,0x18
80101754:  sll v0,v0,0x18
80101758:  sra v0,v0,0x18
8010175c:  sh v1,0xb8(sp)
80101760:  jal 0x8004fbe4
80101764:  _sh v0,0xba(sp)
80101768:  jal 0x8004fdb0
8010176c:  _move a0,s2
80101770:  li v0,0x3
80101774:  sb v0,0xa3(sp)
80101778:  li v0,0x60
8010177c:  sb v0,0xa7(sp)
80101780:  li v0,0xff
80101784:  sb v0,0xa4(sp)
80101788:  li v0,0xc2
8010178c:  sh v0,0xaa(sp)
80101790:  li v0,0xc
80101794:  sb zero,0xa5(sp)
80101798:  sb zero,0xa6(sp)
8010179c:  sh zero,0xa8(sp)
801017a0:  sh v0,0xae(sp)
801017a4:  lw a1,0xc(s4)
801017a8:  addiu a0,sp,0x88
801017ac:  jal 0x800185cc
801017b0:  _addu a1,s4,a1
801017b4:  lw v1,0x94(sp)
801017b8:  li v0,0xdd
801017bc:  sh v0,0x0(v1)
801017c0:  lw v1,0x94(sp)
801017c4:  li v0,0xb6
801017c8:  sh v0,0x2(v1)
801017cc:  lw a0,0x94(sp)
801017d0:  lw a1,0x98(sp)
801017d4:  lui s1,0x52
801017d8:  jal 0x8004f82c
801017dc:  _ori s1,s1,0x5252
801017e0:  clear a0
801017e4:  li a1,0xb9
801017e8:  li a2,0xdb
801017ec:  move a3,a1
801017f0:  jal 0x80019f44
801017f4:  _sw s1,0x10(sp)
801017f8:  clear a0
801017fc:  li a1,0xba
80101800:  li a2,0xdb
80101804:  move a3,a1
80101808:  li s0,0x2988
8010180c:  jal 0x80019f44
80101810:  _sw s0,0x10(sp)
80101814:  clear a0
80101818:  li a1,0xd5
8010181c:  li a2,0xe2
80101820:  move a3,a1
80101824:  jal 0x80019f44
80101828:  _sw s0,0x10(sp)
8010182c:  clear a0
80101830:  li a1,0xd6
80101834:  li a2,0xe3
80101838:  move a3,a1
8010183c:  jal 0x80019f44
80101840:  _sw s1,0x10(sp)
80101844:  addiu s0,sp,0x48
80101848:  move a0,s0
8010184c:  move a1,s3
80101850:  lui v0,0x8006
80101854:  jal 0x800524f4
80101858:  _sw zero,0x5310(v0)
8010185c:  move a0,s0
80101860:  sltiu a0,zero,0x1b51
80101864:  li a1,0x2e
80101868:  lui v1,0x8010
8010186c:  addiu t5,v1,0x58
80101870:  lwl t2,0x3(t5)
80101874:  lwr t2,0x0(t5)
80101878:  lb t3,0x4(t5)
8010187c:  swl t2,0x3(v0)
80101880:  swr t2,0x0(v0)
80101884:  sb t3,0x4(v0)
80101888:  jal 0x80011adc
8010188c:  _move a0,s0
80101890:  move a0,s3
80101894:  li a1,0x5c
80101898:  lui v1,0x8006
8010189c:  sltiu a0,zero,0x1b51
801018a0:  sw v0,0x5a38(v1)
801018a4:  beq v0,zero,0x801018b0
801018a8:  _move a1,s3
801018ac:  addiu a1,v0,0x1
801018b0:  jal 0x800524f4
801018b4:  _move a0,s0
801018b8:  move a0,s0
801018bc:  sltiu a0,zero,0x1b51
801018c0:  li a1,0x2e
801018c4:  bne v0,zero,0x801018d8
801018c8:  _nop
801018cc:  jal 0x80052544
801018d0:  _move a0,s0
801018d4:  addu v0,s0,v0
801018d8:  sb zero,0x0(v0)
801018dc:  clear a0
801018e0:  addiu s0,sp,0x48
801018e4:  move a1,s0
801018e8:  lui v0,0x8006
801018ec:  jal 0x80011aa8
801018f0:  _addiu s1,v0,0x5a34
801018f4:  bne v0,zero,0x80101924
801018f8:  _nop
801018fc:  lui v0,0x8006
80101900:  lw v0,0x5a38(v0)
80101904:  nop
80101908:  lw a0,0x4(v0)
8010190c:  jal 0x80011a38
80101910:  _move a1,s0
80101914:  bne v0,zero,0x80101928
80101918:  _lui s0,0x8006
8010191c:  lui v0,0x8002
80101920:  addiu v0,v0,0x22a8
80101924:  lui s0,0x8006
80101928:  sw v0,0x0(s1)
8010192c:  lw a0,0x5a34(s0)
80101930:  clear a1
80101934:  move a2,a1
80101938:  jal 0x80021b80
8010193c:  _move a3,a1
80101940:  lw a1,0x5a34(s0)
80101944:  move a0,s3
80101948:  lui v1,0x8006
8010194c:  sw v0,0x59fc(v1)
80101950:  jal 0x800159b4
80101954:  _sw a1,0x64(v0)
80101958:  addiu a0,sp,0x18
8010195c:  jal 0x800225d4
80101960:  _addiu a1,sp,0x228
80101964:  lw v0,0x1c(sp)
80101968:  nop
8010196c:  move s3,v0
80101970:  beq s3,zero,0x80101eb8
80101974:  _sw s3,0x228(sp)
80101978:  lui v0,0x8010
8010197c:  addiu s7,v0,0x6d94
80101980:  lui s6,0x8006
80101984:  addiu s5,sp,0x128
80101988:  addiu a0,sp,0x18
8010198c:  jal 0x800225d4
80101990:  _addiu a1,sp,0x228
80101994:  lw a1,0x228(sp)
80101998:  nop
8010199c:  subu a1,s3,a1
801019a0:  sll a0,a1,0x3
801019a4:  subu a0,a0,a1
801019a8:  sll v1,a0,0x5
801019ac:  subu v1,v1,a0
801019b0:  addu v1,v1,a1
801019b4:  div v1,s3
801019b8:  mflo v1
801019bc:  move s2,v0
801019c0:  addiu a0,sp,0xa0
801019c4:  jal 0x8004fb18
801019c8:  _sh v1,0xac(sp)
801019cc:  bne s2,zero,0x80101a44
801019d0:  _lui a1,0x4f42
801019d4:  lw a0,0x18(sp)
801019d8:  ori a1,a1,0x4a20
801019dc:  srl v1,a0,0x18
801019e0:  srl v0,a0,0x8
801019e4:  andi v0,v0,0xff00
801019e8:  or v1,v1,v0
801019ec:  andi v0,a0,0xff00
801019f0:  sll v0,v0,0x8
801019f4:  or v1,v1,v0
801019f8:  sll a0,a0,0x18
801019fc:  or v1,v1,a0
80101a00:  beq v1,a1,0x80101a1c
80101a04:  _lui v0,0x584f
80101a08:  ori v0,v0,0x4246
80101a0c:  beq v1,v0,0x80101a30
80101a10:  _nop
80101a14:  slti a0,zero,0x7aa
80101a18:  nop
80101a1c:  lw a0,0x1c(sp)
80101a20:  sltiu a0,zero,0x3a6
80101a24:  nop
80101a28:  slti a0,zero,0x7aa
80101a2c:  nop
80101a30:  lw a0,0x1c(sp)
80101a34:  sltiu a0,zero,0x102
80101a38:  nop
80101a3c:  slti a0,zero,0x7aa
80101a40:  nop
80101a44:  jal 0x8004f580
80101a48:  _clear a0
80101a4c:  lw a0,0x18(sp)
80101a50:  lui a1,0x5355
80101a54:  ori a1,a1,0x4e41
80101a58:  srl v1,a0,0x18
80101a5c:  srl v0,a0,0x8
80101a60:  andi v0,v0,0xff00
80101a64:  or v1,v1,v0
80101a68:  andi v0,a0,0xff00
80101a6c:  sll v0,v0,0x8
80101a70:  or v1,v1,v0
80101a74:  sll a0,a0,0x18
80101a78:  or v1,v1,a0
80101a7c:  beq v1,a1,0x80101e58
80101a80:  _sltu v0,a1,v1
80101a84:  bne v0,zero,0x80101b34
80101a88:  _lui v0,0x5845
80101a8c:  lui v0,0x4845
80101a90:  ori v0,v0,0x4144
80101a94:  beq v1,v0,0x80101d68
80101a98:  _sltu v0,v0,v1
80101a9c:  bne v0,zero,0x80101ae8
80101aa0:  _lui v0,0x504c
80101aa4:  lui v0,0x4253
80101aa8:  ori v0,v0,0x5020
80101aac:  beq v1,v0,0x80101e44
80101ab0:  _sltu v0,v0,v1
80101ab4:  bne v0,zero,0x80101ad4
80101ab8:  _lui v0,0x434f
80101abc:  lui v0,0x4149
80101ac0:  ori v0,v0,0x4d50
80101ac4:  beq v1,v0,0x80101de0
80101ac8:  _nop
80101acc:  slti a0,zero,0x7a8
80101ad0:  nop
80101ad4:  ori v0,v0,0x4c53
80101ad8:  beq v1,v0,0x80101e6c
80101adc:  _nop
80101ae0:  slti a0,zero,0x7a8
80101ae4:  nop
80101ae8:  ori v0,v0,0x5458
80101aec:  beq v1,v0,0x80101d2c
80101af0:  _sltu v0,v0,v1
80101af4:  bne v0,zero,0x80101b14
80101af8:  _lui v0,0x5245
80101afc:  lui v0,0x4a55
80101b00:  ori v0,v0,0x4e43
80101b04:  beq v1,v0,0x80101e1c
80101b08:  _nop
80101b0c:  slti a0,zero,0x7a8
80101b10:  nop
80101b14:  ori v0,v0,0x4354
80101b18:  beq v1,v0,0x80101df4
80101b1c:  _lui v0,0x5253
80101b20:  ori v0,v0,0x4547
80101b24:  beq v1,v0,0x80101e30
80101b28:  _nop
80101b2c:  slti a0,zero,0x7a8
80101b30:  nop
80101b34:  ori v0,v0,0x4e56
80101b38:  beq v1,v0,0x80101e80
80101b3c:  _sltu v0,v0,v1
80101b40:  bne v0,zero,0x80101b98
80101b44:  _lui v0,0x5852
80101b48:  lui v0,0x5449
80101b4c:  ori v0,v0,0x4e46
80101b50:  beq v1,v0,0x80101da4
80101b54:  _sltu v0,v0,v1
80101b58:  bne v0,zero,0x80101b78
80101b5c:  _lui v0,0x5842
80101b60:  lui v0,0x5445
80101b64:  ori v0,v0,0x5854
80101b68:  beq v1,v0,0x80101d7c
80101b6c:  _nop
80101b70:  slti a0,zero,0x7a8
80101b74:  nop
80101b78:  ori v0,v0,0x474d
80101b7c:  beq v1,v0,0x80101e94
80101b80:  _lui v0,0x5842
80101b84:  ori v0,v0,0x4d50
80101b88:  beq v1,v0,0x80101d90
80101b8c:  _nop
80101b90:  slti a0,zero,0x7a8
80101b94:  nop
80101b98:  ori v0,v0,0x5450
80101b9c:  beq v1,v0,0x80101e08
80101ba0:  _sltu v0,v0,v1
80101ba4:  bne v0,zero,0x80101bc4
80101ba8:  _lui v0,0x5a4d
80101bac:  lui v0,0x584c
80101bb0:  ori v0,v0,0x5343
80101bb4:  beq v1,v0,0x80101be4
80101bb8:  _nop
80101bbc:  slti a0,zero,0x7a8
80101bc0:  nop
80101bc4:  ori v0,v0,0x4150
80101bc8:  beq v1,v0,0x80101dcc
80101bcc:  _lui v0,0x5a4f
80101bd0:  ori v0,v0,0x4e45
80101bd4:  beq v1,v0,0x80101db8
80101bd8:  _nop
80101bdc:  slti a0,zero,0x7a8
80101be0:  nop
80101be4:  lbu v0,0x3(s2)
80101be8:  lw t2,0x260(sp)
80101bec:  nop
80101bf0:  bne v0,t2,0x80101ea0
80101bf4:  _li a1,0x43
80101bf8:  lw a0,0x8(s4)
80101bfc:  jal 0x80019034
80101c00:  _addu a0,s4,a0
80101c04:  move s0,v0
80101c08:  addiu a0,s2,0x4
80101c0c:  clear a1
80101c10:  li a2,0xa
80101c14:  move a3,a1
80101c18:  li v0,0x40
80101c1c:  sb v0,0x4(s0)
80101c20:  sb v0,0x5(s0)
80101c24:  sltiu a0,zero,0x183e
80101c28:  sb v0,0x6(s0)
80101c2c:  lb v0,0x58f8(s6)
80101c30:  nop
80101c34:  sll v0,v0,0x2
80101c38:  addu v0,v0,s7
80101c3c:  lw a1,0x0(v0)
80101c40:  move a0,s0
80101c44:  li a2,0x13
80101c48:  jal 0x80019960
80101c4c:  _li a3,0xe
80101c50:  move a0,s0
80101c54:  jal 0x80019010
80101c58:  _li a1,0x22
80101c5c:  li v0,0x80
80101c60:  sb v0,0x4(s0)
80101c64:  sb v0,0x5(s0)
80101c68:  sb v0,0x6(s0)
80101c6c:  lb v0,0x58f8(s6)
80101c70:  nop
80101c74:  sll v0,v0,0x2
80101c78:  addu v0,v0,s7
80101c7c:  lw a1,0x0(v0)
80101c80:  move a0,s0
80101c84:  li a2,0x10
80101c88:  jal 0x80019960
80101c8c:  _li a3,0xb
80101c90:  jal 0x800190a8
80101c94:  _move a0,s0
80101c98:  lw a0,0x4(s4)
80101c9c:  li a1,0x1
80101ca0:  jal 0x80019034
80101ca4:  _addu a0,s4,a0
80101ca8:  move s0,v0
80101cac:  move a0,s0
80101cb0:  move a1,s8
80101cb4:  move a2,s5
80101cb8:  li a3,0x120
80101cbc:  li v0,0x60
80101cc0:  sb v0,0x4(s0)
80101cc4:  sb v0,0x5(s0)
80101cc8:  li v0,0x28
80101ccc:  jal 0x80019234
80101cd0:  _sb v0,0x6(s0)
80101cd4:  lw v1,0x0(s0)
80101cd8:  nop
80101cdc:  lbu v1,0x7(v1)
80101ce0:  nop
80101ce4:  mult v0,v1
80101ce8:  move a0,s0
80101cec:  move a1,s5
80101cf0:  li a2,0x10
80101cf4:  li a3,0x40
80101cf8:  mflo t2
80101cfc:  subu a3,a3,t2
80101d00:  srl v0,a3,0x1f
80101d04:  addu a3,a3,v0
80101d08:  sra a3,a3,0x1
80101d0c:  jal 0x80019960
80101d10:  _addiu a3,a3,0x73
80101d14:  jal 0x800190a8
80101d18:  _move a0,s0
80101d1c:  jal 0x80045088
80101d20:  _move a0,s4
80101d24:  slti a0,zero,0x7a8
80101d28:  nop
80101d2c:  clear s1
80101d30:  lui v0,0x8007
80101d34:  addiu s0,v0,0x37a0
80101d38:  lw a0,0x0(s0)
80101d3c:  nop
80101d40:  beq a0,zero,0x80101d50
80101d44:  _nop
80101d48:  jal 0x8001a91c
80101d4c:  _nop
80101d50:  addiu s1,s1,0x1
80101d54:  slti v0,s1,0x10
80101d58:  bne v0,zero,0x80101d38
80101d5c:  _addiu s0,s0,0x4
80101d60:  slti a0,zero,0x7a8
80101d64:  nop
80101d68:  lw a1,0x1c(sp)
80101d6c:  sltiu a0,zero,0xab
80101d70:  move a0,s2
80101d74:  slti a0,zero,0x7a8
80101d78:  nop
80101d7c:  lw a1,0x1c(sp)
80101d80:  sltiu a0,zero,0xe8
80101d84:  move a0,s2
80101d88:  slti a0,zero,0x7a8
80101d8c:  nop
80101d90:  lw a1,0x1c(sp)
80101d94:  sltiu a0,zero,0x150b
80101d98:  move a0,s2
80101d9c:  slti a0,zero,0x7a8
80101da0:  nop
80101da4:  lw a1,0x1c(sp)
80101da8:  sltiu a0,zero,0x1554
80101dac:  move a0,s2
80101db0:  slti a0,zero,0x7a8
80101db4:  nop
80101db8:  lw a1,0x1c(sp)
80101dbc:  sltiu a0,zero,0x15fc
80101dc0:  move a0,s2
80101dc4:  slti a0,zero,0x7a8
80101dc8:  nop
80101dcc:  lw a1,0x1c(sp)
80101dd0:  sltiu a0,zero,0x163d
80101dd4:  move a0,s2
80101dd8:  slti a0,zero,0x7a8
80101ddc:  nop
80101de0:  lw a1,0x1c(sp)
80101de4:  sltiu a0,zero,0x17a
80101de8:  move a0,s2
80101dec:  slti a0,zero,0x7a8
80101df0:  nop
80101df4:  lw a1,0x1c(sp)
80101df8:  sltiu a0,zero,0x18f
80101dfc:  move a0,s2
80101e00:  slti a0,zero,0x7a8
80101e04:  nop
80101e08:  lw a1,0x1c(sp)
80101e0c:  sltiu a0,zero,0x12a5
80101e10:  move a0,s2
80101e14:  slti a0,zero,0x7a8
80101e18:  nop
80101e1c:  lw a1,0x1c(sp)
80101e20:  sltiu a0,zero,0x1347
80101e24:  move a0,s2
80101e28:  slti a0,zero,0x7a8
80101e2c:  nop
80101e30:  lw a1,0x1c(sp)
80101e34:  sltiu a0,zero,0x1418
80101e38:  move a0,s2
80101e3c:  slti a0,zero,0x7a8
80101e40:  nop
80101e44:  lw a1,0x1c(sp)
80101e48:  sltiu a0,zero,0x170
80101e4c:  move a0,s2
80101e50:  slti a0,zero,0x7a8
80101e54:  nop
80101e58:  lw a1,0x1c(sp)
80101e5c:  sltiu a0,zero,0x11c
80101e60:  move a0,s2
80101e64:  slti a0,zero,0x7a8
80101e68:  nop
80101e6c:  lw a1,0x1c(sp)
80101e70:  sltiu a0,zero,0x14c6
80101e74:  move a0,s2
80101e78:  slti a0,zero,0x7a8
80101e7c:  nop
80101e80:  lw a1,0x1c(sp)
80101e84:  sltiu a0,zero,0x165
80101e88:  move a0,s2
80101e8c:  slti a0,zero,0x7a8
80101e90:  nop
80101e94:  lw a1,0x1c(sp)
80101e98:  sltiu a0,zero,0x1686
80101e9c:  move a0,s2
80101ea0:  jal 0x80045088
80101ea4:  _move a0,s2
80101ea8:  lw v0,0x228(sp)
80101eac:  nop
80101eb0:  bne v0,zero,0x80101988
80101eb4:  _nop
80101eb8:  jal 0x80015a00
80101ebc:  _lui s2,0x8006
80101ec0:  jal 0x80017e0c
80101ec4:  _addiu s1,s2,0x5ab0
80101ec8:  lh v0,0x5ab0(s2)
80101ecc:  li v1,0x1800
80101ed0:  mult v0,v1
80101ed4:  lh v0,0x2(s1)
80101ed8:  mflo t0
80101edc:  nop
80101ee0:  nop
80101ee4:  mult v0,v1
80101ee8:  addiu s0,sp,0x20
80101eec:  sra v0,t0,0xc
80101ef0:  sh v0,0x20(sp)
80101ef4:  mflo a0
80101ef8:  sra v0,a0,0xc
80101efc:  sh v0,0x2(s0)
80101f00:  lh v0,0x4(s1)
80101f04:  nop
80101f08:  mult v0,v1
80101f0c:  mflo v1
80101f10:  sra v0,v1,0xc
80101f14:  sh v0,0x4(s0)
80101f18:  lui v0,0x8006
80101f1c:  lw a2,0x5b08(v0)
80101f20:  move a1,s0
80101f24:  clear a0
80101f28:  lui v0,0x8006
80101f2c:  jal 0x8001d404
80101f30:  _sh zero,0x59d0(v0)
80101f34:  lui v0,0x8006
80101f38:  lw a2,0x5af8(v0)
80101f3c:  li a0,0x1
80101f40:  lui a1,0x8010
80101f44:  jal 0x8001d404
80101f48:  _addiu a1,a1,0xe8
80101f4c:  lui v0,0x8006
80101f50:  lw a2,0x5b10(v0)
80101f54:  lhu v0,0x5ab0(s2)
80101f58:  lhu v1,0x4(s1)
80101f5c:  lhu a3,0x2(s1)
80101f60:  li a0,0x2
80101f64:  move a1,s0
80101f68:  subu v0,zero,v0
80101f6c:  subu v1,zero,v1
80101f70:  sh v0,0x20(sp)
80101f74:  sh a3,0x22(sp)
80101f78:  jal 0x8001d404
80101f7c:  _sh v1,0x24(sp)
80101f80:  jal 0x80016da8
80101f84:  _addiu a0,sp,0x28
80101f88:  lui a0,0x8007
80101f8c:  jal 0x8004d3a4
80101f90:  _addiu a0,a0,-0x8a0
80101f94:  lui a0,0x8007
80101f98:  jal 0x8004d374
80101f9c:  _addiu a0,a0,-0x8e0
80101fa0:  li a0,0x40
80101fa4:  move a1,a0
80101fa8:  jal 0x8004d4e4
80101fac:  _move a2,a0
80101fb0:  lui v0,0x8010
80101fb4:  lw s0,0x7da0(v0)
80101fb8:  nop
80101fbc:  lw v0,0x0(s0)
80101fc0:  nop
80101fc4:  beq v0,zero,0x80101fe4
80101fc8:  _clear s3
80101fcc:  lw s0,0x0(s0)
80101fd0:  nop
80101fd4:  lw v0,0x0(s0)
80101fd8:  nop
80101fdc:  bne v0,zero,0x80101fcc
80101fe0:  _addiu s3,s3,0x1
80101fe4:  lui a1,0x8010
80101fe8:  addiu a0,a1,0x7da0
80101fec:  lw v1,0x8(a0)
80101ff0:  li v0,0xff
80101ff4:  sw s3,0x228(sp)
80101ff8:  sb zero,0xa4(sp)
80101ffc:  sb v0,0xa5(sp)
80102000:  beq v1,a0,0x80102090
80102004:  _sb zero,0xa6(sp)
80102008:  move s2,a1
8010200c:  move s1,a0
80102010:  lw v0,0x7da0(s2)
80102014:  nop
80102018:  lw v1,0x0(v0)
8010201c:  move s0,v0
80102020:  sw s1,0x4(v1)
80102024:  sw v1,0x7da0(s2)
80102028:  lw a0,0x8(s0)
8010202c:  sltiu a0,zero,0x55d
80102030:  addiu a1,sp,0x28
80102034:  lui v0,0x8006
80102038:  lw a0,0x5a00(v0)
8010203c:  sltiu a0,zero,0x7b
80102040:  move a1,s0
80102044:  lw a1,0x228(sp)
80102048:  nop
8010204c:  addiu a1,a1,-0x1
80102050:  subu a0,s3,a1
80102054:  sll v1,a0,0x3
80102058:  subu v1,v1,a0
8010205c:  sll v0,v1,0x5
80102060:  subu v0,v0,v1
80102064:  addu v0,v0,a0
80102068:  div v0,s3
8010206c:  mflo v0
80102070:  addiu a0,sp,0xa0
80102074:  sw a1,0x228(sp)
80102078:  jal 0x8004fb18
8010207c:  _sh v0,0xac(sp)
80102080:  lw v0,0x8(s1)
80102084:  nop
80102088:  bne v0,s1,0x80102010
8010208c:  _nop
80102090:  lui a0,0x8010
80102094:  jal 0x80020658
80102098:  _addiu a0,a0,0x7d90
8010209c:  sltiu a0,zero,0x1472
801020a0:  nop
801020a4:  lui v0,0x8006
801020a8:  lb v0,0x5319(v0)
801020ac:  nop
801020b0:  bne v0,zero,0x801021fc
801020b4:  _lui s0,0x8006
801020b8:  lui v0,0x8006
801020bc:  lw a0,0x590c(v0)
801020c0:  lui v0,0x8006
801020c4:  lb v0,0x5674(v0)
801020c8:  lui v1,0x8006
801020cc:  lb v1,0x5904(v1)
801020d0:  sll v0,v0,0x3
801020d4:  addu a0,a0,v0
801020d8:  lw v0,0x8(a0)
801020dc:  sll v1,v1,0x4
801020e0:  addu v1,v1,v0
801020e4:  lw s1,0x8(v1)
801020e8:  clear s2
801020ec:  move s3,s2
801020f0:  lui s4,0x8006
801020f4:  lui v0,0x8006
801020f8:  sb zero,0x5acc(v0)
801020fc:  lui v0,0x8006
80102100:  lw a0,0x590c(v0)
80102104:  lui v0,0x8006
80102108:  lb v1,0x5674(v0)
8010210c:  lui v0,0x8006
80102110:  lb v0,0x5904(v0)
80102114:  sll v1,v1,0x3
80102118:  addu a0,a0,v1
8010211c:  lw v1,0x8(a0)
80102120:  sll v0,v0,0x4
80102124:  addu v0,v0,v1
80102128:  lhu v0,0x6(v0)
8010212c:  nop
80102130:  slt v0,s3,v0
80102134:  beq v0,zero,0x801021f8
80102138:  _li v0,-0x1
8010213c:  lb s0,0x0(s1)
80102140:  nop
80102144:  bne s0,v0,0x80102174
80102148:  _andi v0,s0,0x80
8010214c:  lw v0,0x5ad4(s4)
80102150:  nop
80102154:  bne v0,zero,0x801021e8
80102158:  _nop
8010215c:  jal 0x80021f30
80102160:  _move a0,s1
80102164:  move a0,v0
80102168:  sw a0,0x5ad4(s4)
8010216c:  slti a0,zero,0x878
80102170:  sh s0,0x6(a0)
80102174:  beq v0,zero,0x801021a0
80102178:  _lui v0,0x8006
8010217c:  addiu t5,v0,0x5acc
80102180:  lwl t2,0x3(s1)
80102184:  lwr t2,0x0(s1)
80102188:  lh t3,0x4(s1)
8010218c:  swl t2,0x3(t5)
80102190:  swr t2,0x0(t5)
80102194:  sh t3,0x4(t5)
80102198:  slti a0,zero,0x87b
8010219c:  addiu s3,s3,0x1
801021a0:  jal 0x80021f30
801021a4:  _move a0,s1
801021a8:  move s0,v0
801021ac:  beq s0,zero,0x801021e8
801021b0:  _li v0,0x4
801021b4:  sb v0,0x8(s0)
801021b8:  lui v0,0x8006
801021bc:  lb v0,0x531a(v0)
801021c0:  addiu s2,s2,0x1
801021c4:  bne v0,zero,0x801021dc
801021c8:  _sh s2,0x6(s0)
801021cc:  lhu a1,0xc(s0)
801021d0:  move a0,s0
801021d4:  jal 0x8002e604
801021d8:  _srl a1,a1,0x1
801021dc:  move a0,s0
801021e0:  jal 0x8002036c
801021e4:  _nop
801021e8:  addiu s3,s3,0x1
801021ec:  slti v0,s2,0x6
801021f0:  bne v0,zero,0x801020fc
801021f4:  _addiu s1,s1,0x6
801021f8:  lui s0,0x8006
801021fc:  lw v0,0x5ad4(s0)
80102200:  nop
80102204:  bne v0,zero,0x80102224
80102208:  _lui v0,0x8006
8010220c:  jal 0x80021fd8
80102210:  _li a0,-0x1
80102214:  sw v0,0x5ad4(s0)
80102218:  jal 0x8002036c
8010221c:  _move a0,v0
80102220:  lui v0,0x8006
80102224:  lb v0,0x5319(v0)
80102228:  nop
8010222c:  slti v0,v0,0x3
80102230:  bne v0,zero,0x8010225c
80102234:  _lui s0,0x8006
80102238:  lw v0,0x5ad8(s0)
8010223c:  nop
80102240:  bne v0,zero,0x8010225c
80102244:  _nop
80102248:  jal 0x80021fd8
8010224c:  _li a0,-0x2
80102250:  sw v0,0x5ad8(s0)
80102254:  jal 0x8002036c
80102258:  _move a0,v0
8010225c:  jal 0x8004410c
80102260:  _lui s0,0x8006
80102264:  lw v1,0x5ad4(s0)
80102268:  nop
8010226c:  sb v0,0x5(v1)
80102270:  lui v1,0x8007
80102274:  lw v1,0x37e0(v1)
80102278:  nop
8010227c:  lw a1,0x8(v1)
80102280:  clear a2
80102284:  move a3,a2
80102288:  sll v0,v0,0x18
8010228c:  jal 0x800443c8
80102290:  _sra a0,v0,0x18
80102294:  jal 0x80017160
80102298:  _addiu s1,s0,0x5ad4
8010229c:  lui v1,0x8006
801022a0:  lbu v1,0x5bfc(v1)
801022a4:  nop
801022a8:  mult v0,v1
801022ac:  mflo t2
801022b0:  jal 0x80043ce0
801022b4:  _sra a0,t2,0xf
801022b8:  lw a0,0x5ad4(s0)
801022bc:  jal 0x8002ea94
801022c0:  _li a1,0x1
801022c4:  lw a0,0x5ad4(s0)
801022c8:  jal 0x8003d918
801022cc:  _li a1,0x100
801022d0:  lw v1,0x5ad4(s0)
801022d4:  move a0,v0
801022d8:  jal 0x80020744
801022dc:  _sw a0,0xe0(v1)
801022e0:  lw v0,0x5ad4(s0)
801022e4:  nop
801022e8:  lw a0,0xe0(v0)
801022ec:  jal 0x8003da98
801022f0:  _nop
801022f4:  lui v0,0x8006
801022f8:  lb v1,0x531a(v0)
801022fc:  nop
80102300:  slti v0,v1,0x2
80102304:  beq v0,zero,0x80102340
80102308:  _nop
8010230c:  lw v0,0x5ad4(s0)
80102310:  nop
80102314:  lhu a1,0xc(v0)
80102318:  bne v1,zero,0x80102328
8010231c:  _move a0,s1
80102320:  slti a0,zero,0x8cd
80102324:  sll a1,a1,0x1
80102328:  sll v0,a1,0x1
8010232c:  addu v0,v0,a1
80102330:  sra a1,v0,0x1
80102334:  lw a0,0x0(a0)
80102338:  jal 0x8002e604
8010233c:  _nop
80102340:  lui s0,0x8006
80102344:  lw a0,0x5ad8(s0)
80102348:  nop
8010234c:  beq a0,zero,0x80102404
80102350:  _nop
80102354:  jal 0x8002ea94
80102358:  _li a1,0x1
8010235c:  lw a0,0x5ad8(s0)
80102360:  jal 0x8003d918
80102364:  _li a1,0x100
80102368:  lw v1,0x5ad8(s0)
8010236c:  move a0,v0
80102370:  jal 0x80020744
80102374:  _sw a0,0xe0(v1)
80102378:  lw v0,0x5ad8(s0)
8010237c:  nop
80102380:  lw a0,0xe0(v0)
80102384:  jal 0x8003da98
80102388:  _nop
8010238c:  lui v0,0x8006
80102390:  lw v1,0x5ad4(v0)
80102394:  lw v0,0x5ad8(s0)
80102398:  nop
8010239c:  sw v0,0xe4(v1)
801023a0:  jal 0x8004410c
801023a4:  _sw v1,0xe4(v0)
801023a8:  lw v1,0x5ad8(s0)
801023ac:  nop
801023b0:  sb v0,0x5(v1)
801023b4:  lui v1,0x8007
801023b8:  lw v1,0x37e4(v1)
801023bc:  nop
801023c0:  lw a1,0x8(v1)
801023c4:  clear a2
801023c8:  move a3,a2
801023cc:  sll v0,v0,0x18
801023d0:  jal 0x800443c8
801023d4:  _sra a0,v0,0x18
801023d8:  lui v0,0x8006
801023dc:  lb v0,0x531a(v0)
801023e0:  nop
801023e4:  slti v0,v0,0x2
801023e8:  beq v0,zero,0x80102404
801023ec:  _nop
801023f0:  lw a0,0x5ad8(s0)
801023f4:  nop
801023f8:  lhu a1,0xc(a0)
801023fc:  jal 0x8002e604
80102400:  _sll a1,a1,0x1
80102404:  lw ra,0x254(sp)
80102408:  lw s8,0x250(sp)
8010240c:  lw s7,0x24c(sp)
80102410:  lw s6,0x248(sp)
80102414:  lw s5,0x244(sp)
80102418:  lw s4,0x240(sp)
8010241c:  lw s3,0x23c(sp)
80102420:  lw s2,0x238(sp)
80102424:  lw s1,0x234(sp)
80102428:  lw s0,0x230(sp)
8010242c:  lui v0,0x8006
80102430:  sb zero,0x5980(v0)
80102434:  jr ra
80102438:  _addiu sp,sp,0x258
