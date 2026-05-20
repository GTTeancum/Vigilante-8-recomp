# addr: 0x8010c784  name: FUN_8010c784
8010c784:  addiu sp,sp,-0x78
8010c788:  li a0,0x140
8010c78c:  li a1,0xf0
8010c790:  li a2,0xa0
8010c794:  lui v1,0x8006
8010c798:  li v0,0x1
8010c79c:  li a3,0x78
8010c7a0:  sw ra,0x74(sp)
8010c7a4:  sw s8,0x70(sp)
8010c7a8:  sw s7,0x6c(sp)
8010c7ac:  sw s6,0x68(sp)
8010c7b0:  sw s5,0x64(sp)
8010c7b4:  sw s4,0x60(sp)
8010c7b8:  sw s3,0x5c(sp)
8010c7bc:  sw s2,0x58(sp)
8010c7c0:  sw s1,0x54(sp)
8010c7c4:  sw s0,0x50(sp)
8010c7c8:  jal 0x8001d994
8010c7cc:  _sw v0,0x532c(v1)
8010c7d0:  lui v0,0x8006
8010c7d4:  lb v0,0x5318(v0)
8010c7d8:  nop
8010c7dc:  bne v0,zero,0x8010c8f8
8010c7e0:  _lui s0,0x8006
8010c7e4:  li v1,0x1
8010c7e8:  li s0,0xb
8010c7ec:  lui v0,0x8006
8010c7f0:  addiu v0,v0,0x5950
8010c7f4:  addu v0,v0,s0
8010c7f8:  sb v1,0x0(v0)
8010c7fc:  addiu s0,s0,-0x1
8010c800:  bgez s0,0x8010c7f8
8010c804:  _addiu v0,v0,-0x1
8010c808:  sltiu a0,zero,0x3b98
8010c80c:  clear a0
8010c810:  jal 0x80011f0c
8010c814:  _lui s1,0x8006
8010c818:  sltiu a0,zero,0x2b5f
8010c81c:  addiu s2,s1,0x5930
8010c820:  lui a0,0x8010
8010c824:  addiu a0,a0,0x1000
8010c828:  li a1,0x1
8010c82c:  sltiu a0,zero,0x34d5
8010c830:  lui a2,0x840
8010c834:  lw v0,0x5930(s1)
8010c838:  lw v1,0x4(s2)
8010c83c:  nop
8010c840:  or v0,v0,v1
8010c844:  lui v1,0x800
8010c848:  and v0,v0,v1
8010c84c:  bne v0,zero,0x8010c8ec
8010c850:  _lui v1,0x8006
8010c854:  sltiu a0,zero,0x8e3
8010c858:  clear s0
8010c85c:  lw v0,0x5930(s1)
8010c860:  lw v1,0x4(s2)
8010c864:  nop
8010c868:  or v0,v0,v1
8010c86c:  lui v1,0x840
8010c870:  and v0,v0,v1
8010c874:  bne v0,zero,0x8010c8bc
8010c878:  _nop
8010c87c:  move s3,s1
8010c880:  move s1,v1
8010c884:  jal 0x800126f0
8010c888:  _addiu s0,s0,0x1
8010c88c:  jal 0x80047e44
8010c890:  _clear a0
8010c894:  slti v0,s0,0x78
8010c898:  beq v0,zero,0x8010c8bc
8010c89c:  _nop
8010c8a0:  lw v0,0x5930(s3)
8010c8a4:  lw v1,0x4(s2)
8010c8a8:  nop
8010c8ac:  or v0,v0,v1
8010c8b0:  and v0,v0,s1
8010c8b4:  beq v0,zero,0x8010c884
8010c8b8:  _nop
8010c8bc:  jal 0x80015fb4
8010c8c0:  _nop
8010c8c4:  lui v0,0x8006
8010c8c8:  lw v1,0x5930(v0)
8010c8cc:  addiu v0,v0,0x5930
8010c8d0:  lw v0,0x4(v0)
8010c8d4:  nop
8010c8d8:  or v1,v1,v0
8010c8dc:  lui v0,0x800
8010c8e0:  and v1,v1,v0
8010c8e4:  beq v1,zero,0x8010c8f4
8010c8e8:  _lui v1,0x8006
8010c8ec:  li v0,0x3
8010c8f0:  sb v0,0x5318(v1)
8010c8f4:  lui s0,0x8006
8010c8f8:  lb v1,0x5318(s0)
8010c8fc:  nop
8010c900:  slti v0,v1,0x2
8010c904:  beq v0,zero,0x8010c930
8010c908:  _li v0,0x2
8010c90c:  jal 0x800165cc
8010c910:  _clear a0
8010c914:  lui a0,0x8010
8010c918:  addiu a0,a0,0x1014
8010c91c:  li a1,0x1
8010c920:  sltiu a0,zero,0x34d5
8010c924:  lui a2,0x840
8010c928:  slti a0,zero,0x3261
8010c92c:  lui v0,0x8006
8010c930:  bne v1,v0,0x8010c980
8010c934:  _nop
8010c938:  jal 0x800165cc
8010c93c:  _clear a0
8010c940:  lui a0,0x8010
8010c944:  addiu a0,a0,0x1014
8010c948:  li a1,0x1
8010c94c:  sltiu a0,zero,0x34d5
8010c950:  li a2,-0x1
8010c954:  lui v0,0x8006
8010c958:  lw v1,0x5930(v0)
8010c95c:  nop
8010c960:  bne v1,zero,0x8010c978
8010c964:  _addiu v0,v0,0x5930
8010c968:  lw v0,0x4(v0)
8010c96c:  nop
8010c970:  beq v0,zero,0x8010c984
8010c974:  _lui v0,0x8006
8010c978:  li v0,0x4
8010c97c:  sb v0,0x5318(s0)
8010c980:  lui v0,0x8006
8010c984:  lb v0,0x5318(v0)
8010c988:  nop
8010c98c:  slti v0,v0,0x4
8010c990:  beq v0,zero,0x8010ca6c
8010c994:  _nop
8010c998:  lui a0,0x8010
8010c99c:  jal 0x80015f80
8010c9a0:  _addiu a0,a0,0x1024
8010c9a4:  lui a0,0x8010
8010c9a8:  addiu a0,a0,0x1038
8010c9ac:  lui v1,0x8011
8010c9b0:  jal 0x80015f80
8010c9b4:  _sw v0,0x3388(v1)
8010c9b8:  jal 0x8001a8fc
8010c9bc:  _move a0,v0
8010c9c0:  lui a0,0x8010
8010c9c4:  addiu a0,a0,0x104c
8010c9c8:  lui v1,0x8011
8010c9cc:  jal 0x80044360
8010c9d0:  _sw v0,0x3390(v1)
8010c9d4:  clear a0
8010c9d8:  lui v1,0x8011
8010c9dc:  jal 0x800165cc
8010c9e0:  _sw v0,0x338c(v1)
8010c9e4:  clear s0
8010c9e8:  lui s2,0x8010
8010c9ec:  lui s1,0x6666
8010c9f0:  ori s1,s1,0x6667
8010c9f4:  sltiu a0,zero,0x92f
8010c9f8:  nop
8010c9fc:  bne v0,zero,0x8010ca48
8010ca00:  _addiu a0,sp,0x10
8010ca04:  addiu a1,s2,0x1060
8010ca08:  jal 0x80053004
8010ca0c:  _addiu a2,s0,0x31
8010ca10:  addiu a0,sp,0x10
8010ca14:  li a1,0x1
8010ca18:  sltiu a0,zero,0x34d5
8010ca1c:  li a2,-0x1
8010ca20:  addiu a0,s0,0x1
8010ca24:  mult a0,s1
8010ca28:  sra v0,a0,0x1f
8010ca2c:  mfhi t0
8010ca30:  sra v1,t0,0x1
8010ca34:  subu s0,v1,v0
8010ca38:  sll v0,s0,0x2
8010ca3c:  addu v0,v0,s0
8010ca40:  slti a0,zero,0x327d
8010ca44:  subu s0,a0,v0
8010ca48:  lui v0,0x8011
8010ca4c:  lw a1,0x338c(v0)
8010ca50:  li a0,0x1
8010ca54:  jal 0x8004445c
8010ca58:  _li a2,0x8
8010ca5c:  jal 0x80015fb4
8010ca60:  _nop
8010ca64:  slti a0,zero,0x32ad
8010ca68:  lui v0,0x8006
8010ca6c:  lui a0,0x8010
8010ca70:  jal 0x80015f80
8010ca74:  _addiu a0,a0,0x1024
8010ca78:  lui a0,0x8010
8010ca7c:  addiu a0,a0,0x1038
8010ca80:  lui v1,0x8011
8010ca84:  jal 0x80015f80
8010ca88:  _sw v0,0x3388(v1)
8010ca8c:  jal 0x8001a8fc
8010ca90:  _move a0,v0
8010ca94:  lui a0,0x8010
8010ca98:  addiu a0,a0,0x104c
8010ca9c:  lui v1,0x8011
8010caa0:  jal 0x80044360
8010caa4:  _sw v0,0x3390(v1)
8010caa8:  lui v1,0x8011
8010caac:  sw v0,0x338c(v1)
8010cab0:  lui v0,0x8006
8010cab4:  lb v1,0x5318(v0)
8010cab8:  li v0,0x4
8010cabc:  bne v1,v0,0x8010cc8c
8010cac0:  _lui v0,0x8006
8010cac4:  lb v0,0x5319(v0)
8010cac8:  nop
8010cacc:  bne v0,zero,0x8010cc8c
8010cad0:  _lui v0,0x8006
8010cad4:  lw v0,0x5328(v0)
8010cad8:  nop
8010cadc:  beq v0,zero,0x8010cc3c
8010cae0:  _lui s3,0x8006
8010cae4:  lb v0,0x5674(s3)
8010cae8:  lui v1,0x8006
8010caec:  addiu s2,v1,0x5950
8010caf0:  addu v0,v0,s2
8010caf4:  lbu s4,0x0(v0)
8010caf8:  lui s1,0x8006
8010cafc:  lb s0,0x5904(s1)
8010cb00:  sltiu a0,zero,0x384d
8010cb04:  move a0,s4
8010cb08:  li v1,0x1f
8010cb0c:  subu v1,v1,v0
8010cb10:  bne s0,v1,0x8010cbac
8010cb14:  _lui v0,0x8006
8010cb18:  lb a0,0x5674(s3)
8010cb1c:  lb a2,0x5904(s1)
8010cb20:  li a1,0x1
8010cb24:  li v1,0x2
8010cb28:  addu a0,a0,s2
8010cb2c:  sllv v0,a1,a2
8010cb30:  nor v0,zero,v0
8010cb34:  and v0,s4,v0
8010cb38:  sllv v1,v1,a2
8010cb3c:  or v0,v0,v1
8010cb40:  sb v0,0x0(a0)
8010cb44:  lb a0,0x5674(s3)
8010cb48:  lui v0,0x8006
8010cb4c:  lw v0,0x590c(v0)
8010cb50:  sll v1,a0,0x3
8010cb54:  addu v0,v0,v1
8010cb58:  lw v1,0x4(v0)
8010cb5c:  addu a0,a0,s2
8010cb60:  lbu v0,0x0(a0)
8010cb64:  sllv a1,a1,v1
8010cb68:  slt v0,v0,a1
8010cb6c:  bne v0,zero,0x8010cbac
8010cb70:  _lui v0,0x8006
8010cb74:  jal 0x800165cc
8010cb78:  _clear a0
8010cb7c:  lb a0,0x5674(s3)
8010cb80:  lui v1,0x8011
8010cb84:  addiu v1,v1,0x1c68
8010cb88:  sll v0,a0,0x2
8010cb8c:  addu v0,v0,a0
8010cb90:  sll v0,v0,0x2
8010cb94:  addu v0,v0,v1
8010cb98:  lw a0,0x4(v0)
8010cb9c:  li a1,0x1
8010cba0:  sltiu a0,zero,0x34d5
8010cba4:  lui a2,0x840
8010cba8:  lui v0,0x8006
8010cbac:  lw v0,0x5924(v0)
8010cbb0:  nop
8010cbb4:  beq v0,zero,0x8010cbe8
8010cbb8:  _lui v0,0x8006
8010cbbc:  lb a0,0x5674(v0)
8010cbc0:  lui v0,0x8006
8010cbc4:  lb a1,0x5904(v0)
8010cbc8:  lui v0,0x8006
8010cbcc:  addiu v0,v0,0x5950
8010cbd0:  addu a0,a0,v0
8010cbd4:  lbu v1,0x0(a0)
8010cbd8:  li v0,0x1
8010cbdc:  sllv v0,v0,a1
8010cbe0:  or v1,v1,v0
8010cbe4:  sb v1,0x0(a0)
8010cbe8:  lui v0,0x8006
8010cbec:  lb v1,0x5674(v0)
8010cbf0:  lui v0,0x8006
8010cbf4:  addiu v0,v0,0x5950
8010cbf8:  addu v0,v1,v0
8010cbfc:  lbu a0,0x0(v0)
8010cc00:  nop
8010cc04:  beq a0,s4,0x8010cc3c
8010cc08:  _lui v0,0x8006
8010cc0c:  lw v0,0x590c(v0)
8010cc10:  sll v1,v1,0x3
8010cc14:  addu v0,v0,v1
8010cc18:  lw v1,0x4(v0)
8010cc1c:  li v0,0x2
8010cc20:  sllv v0,v0,v1
8010cc24:  addiu v0,v0,-0x1
8010cc28:  bne a0,v0,0x8010cc40
8010cc2c:  _lui v0,0x8006
8010cc30:  lui v1,0x8006
8010cc34:  li v0,-0x1
8010cc38:  sb v0,0x5319(v1)
8010cc3c:  lui v0,0x8006
8010cc40:  lb v0,0x5319(v0)
8010cc44:  nop
8010cc48:  bne v0,zero,0x8010cc8c
8010cc4c:  _lui v0,0x8006
8010cc50:  lb a0,0x5674(v0)
8010cc54:  sltiu a0,zero,0xd51
8010cc58:  nop
8010cc5c:  li a0,0x1
8010cc60:  lui v1,0x8006
8010cc64:  jal 0x80016678
8010cc68:  _sb v0,0x58f8(v1)
8010cc6c:  sltiu a0,zero,0x31a4
8010cc70:  nop
8010cc74:  lui v0,0x8006
8010cc78:  lw v0,0x5930(v0)
8010cc7c:  lui v1,0x840
8010cc80:  and v0,v0,v1
8010cc84:  bne v0,zero,0x8010cf58
8010cc88:  _lui v0,0x8011
8010cc8c:  sltiu a0,zero,0x4be
8010cc90:  lui s0,0x8011
8010cc94:  sw v0,0x33b4(s0)
8010cc98:  lui a0,0x8006
8010cc9c:  jal 0x80011be4
8010cca0:  _addiu a0,a0,0x5968
8010cca4:  lui v0,0x8010
8010cca8:  addiu s8,v0,0x1078
8010ccac:  lui s3,0x8006
8010ccb0:  addiu s7,s3,0x5674
8010ccb4:  lui s1,0x8006
8010ccb8:  lui s5,0x8006
8010ccbc:  lui s4,0x8006
8010ccc0:  move s2,s0
8010ccc4:  lui a0,0x8006
8010ccc8:  jal 0x80011c58
8010cccc:  _addiu a0,a0,0x5968
8010ccd0:  sltiu a0,zero,0x99b
8010ccd4:  nop
8010ccd8:  li a0,0x1
8010ccdc:  jal 0x80016678
8010cce0:  _move s6,v0
8010cce4:  sltiu v0,s6,0x5
8010cce8:  beq v0,zero,0x8010cf14
8010ccec:  _sll v0,s6,0x2
8010ccf0:  addu v0,v0,s8
8010ccf4:  lw v0,0x0(v0)
8010ccf8:  nop
8010ccfc:  jr v0
8010cd00:  _nop
8010cd04:  lw a0,0x33b4(s2)
8010cd08:  clear a1
8010cd0c:  sb zero,0x5319(s4)
8010cd10:  sltiu a0,zero,0x1349
8010cd14:  ori a0,a0,0x1000
8010cd18:  li a0,0x1
8010cd1c:  sb v0,0x5674(s3)
8010cd20:  li v0,-0x1
8010cd24:  jal 0x80016678
8010cd28:  _sb v0,0x1(s7)
8010cd2c:  lw v0,0x5930(s1)
8010cd30:  lui v1,0x90
8010cd34:  and v0,v0,v1
8010cd38:  bne v0,zero,0x8010cf14
8010cd3c:  _nop
8010cd40:  lb a0,0x5674(s3)
8010cd44:  sltiu a0,zero,0xd51
8010cd48:  nop
8010cd4c:  li a0,0x1
8010cd50:  jal 0x80016678
8010cd54:  _sb v0,0x58f8(s5)
8010cd58:  sltiu a0,zero,0x31a4
8010cd5c:  nop
8010cd60:  lw v0,0x5930(s1)
8010cd64:  lui v1,0x10
8010cd68:  and v0,v0,v1
8010cd6c:  beq v0,zero,0x8010cf14
8010cd70:  _nop
8010cd74:  slti a0,zero,0x3341
8010cd78:  nop
8010cd7c:  lui v0,0x8006
8010cd80:  addiu s0,v0,0x5930
8010cd84:  lw a0,0x33b4(s2)
8010cd88:  li v0,0x1
8010cd8c:  sltiu a0,zero,0xaf7
8010cd90:  sb v0,0x5319(s4)
8010cd94:  li a0,0x1
8010cd98:  jal 0x80016678
8010cd9c:  _sb v0,0x58f8(s5)
8010cda0:  lw v0,0x5930(s1)
8010cda4:  lw v1,0x4(s0)
8010cda8:  nop
8010cdac:  or v0,v0,v1
8010cdb0:  lui v1,0x90
8010cdb4:  and v0,v0,v1
8010cdb8:  bne v0,zero,0x8010cf14
8010cdbc:  _nop
8010cdc0:  lw a0,0x33b4(s2)
8010cdc4:  sltiu a0,zero,0x1349
8010cdc8:  li a1,0x1
8010cdcc:  li a0,0x1
8010cdd0:  sb v0,0x5674(s3)
8010cdd4:  li v0,-0x1
8010cdd8:  jal 0x80016678
8010cddc:  _sb v0,0x1(s7)
8010cde0:  lw v0,0x5930(s1)
8010cde4:  lui v1,0x10
8010cde8:  and v0,v0,v1
8010cdec:  beq v0,zero,0x8010cf14
8010cdf0:  _nop
8010cdf4:  slti a0,zero,0x3361
8010cdf8:  nop
8010cdfc:  lui v0,0x8006
8010ce00:  addiu s0,v0,0x5930
8010ce04:  lw a0,0x33b4(s2)
8010ce08:  li v0,0x3
8010ce0c:  sltiu a0,zero,0xaf7
8010ce10:  sb v0,0x5319(s4)
8010ce14:  li a0,0x1
8010ce18:  jal 0x80016678
8010ce1c:  _sb v0,0x58f8(s5)
8010ce20:  lw v0,0x5930(s1)
8010ce24:  lw v1,0x4(s0)
8010ce28:  nop
8010ce2c:  or v0,v0,v1
8010ce30:  lui v1,0x90
8010ce34:  and v0,v0,v1
8010ce38:  bne v0,zero,0x8010cf14
8010ce3c:  _nop
8010ce40:  lw a0,0x33b4(s2)
8010ce44:  sltiu a0,zero,0x1598
8010ce48:  nop
8010ce4c:  li a0,0x1
8010ce50:  jal 0x80016678
8010ce54:  _sh v0,0x5674(s3)
8010ce58:  lw v0,0x5930(s1)
8010ce5c:  lw v1,0x4(s0)
8010ce60:  nop
8010ce64:  or v0,v0,v1
8010ce68:  lui v1,0x10
8010ce6c:  and v0,v0,v1
8010ce70:  beq v0,zero,0x8010cf14
8010ce74:  _nop
8010ce78:  slti a0,zero,0x3381
8010ce7c:  nop
8010ce80:  lui v0,0x8006
8010ce84:  addiu s0,v0,0x5930
8010ce88:  lw a0,0x33b4(s2)
8010ce8c:  li v0,0x4
8010ce90:  sltiu a0,zero,0xaf7
8010ce94:  sb v0,0x5319(s4)
8010ce98:  li a0,0x1
8010ce9c:  jal 0x80016678
8010cea0:  _sb v0,0x58f8(s5)
8010cea4:  lw v0,0x5930(s1)
8010cea8:  lw v1,0x4(s0)
8010ceac:  nop
8010ceb0:  or v0,v0,v1
8010ceb4:  lui v1,0x90
8010ceb8:  and v0,v0,v1
8010cebc:  bne v0,zero,0x8010cf14
8010cec0:  _nop
8010cec4:  lw a0,0x33b4(s2)
8010cec8:  sltiu a0,zero,0x18f9
8010cecc:  nop
8010ced0:  li a0,0x1
8010ced4:  jal 0x80016678
8010ced8:  _sh v0,0x5674(s3)
8010cedc:  lw v0,0x5930(s1)
8010cee0:  lw v1,0x4(s0)
8010cee4:  nop
8010cee8:  or v0,v0,v1
8010ceec:  lui v1,0x10
8010cef0:  and v0,v0,v1
8010cef4:  beq v0,zero,0x8010cf14
8010cef8:  _nop
8010cefc:  slti a0,zero,0x33a2
8010cf00:  nop
8010cf04:  sltiu a0,zero,0x3071
8010cf08:  nop
8010cf0c:  jal 0x80016678
8010cf10:  _li a0,0x1
8010cf14:  jal 0x80011834
8010cf18:  _nop
8010cf1c:  lw v1,0x5930(s1)
8010cf20:  addiu v0,s1,0x5930
8010cf24:  lw v0,0x4(v0)
8010cf28:  nop
8010cf2c:  or v1,v1,v0
8010cf30:  lui v0,0x90
8010cf34:  and v1,v1,v0
8010cf38:  bne v1,zero,0x8010ccc8
8010cf3c:  _lui a0,0x8006
8010cf40:  bltz s6,0x8010cf54
8010cf44:  _lui v1,0x8006
8010cf48:  addiu v0,v1,0x5978
8010cf4c:  sw zero,0x4(v0)
8010cf50:  sw zero,0x5978(v1)
8010cf54:  lui v0,0x8011
8010cf58:  lw a0,0x3390(v0)
8010cf5c:  jal 0x8001aa38
8010cf60:  _nop
8010cf64:  lui v0,0x8011
8010cf68:  lw a0,0x3388(v0)
8010cf6c:  jal 0x80045088
8010cf70:  _nop
8010cf74:  lui v0,0x8011
8010cf78:  lw a0,0x338c(v0)
8010cf7c:  jal 0x80044394
8010cf80:  _nop
8010cf84:  lui v0,0x8006
8010cf88:  lb a0,0x58f8(v0)
8010cf8c:  lw ra,0x74(sp)
8010cf90:  lw s8,0x70(sp)
8010cf94:  lw s7,0x6c(sp)
8010cf98:  lw s6,0x68(sp)
8010cf9c:  lw s5,0x64(sp)
8010cfa0:  lw s4,0x60(sp)
8010cfa4:  lw s3,0x5c(sp)
8010cfa8:  lw s2,0x58(sp)
8010cfac:  lw s1,0x54(sp)
8010cfb0:  lui v0,0x8006
8010cfb4:  lui v1,0x8006
8010cfb8:  sw zero,0x532c(v0)
8010cfbc:  li v0,0x4
8010cfc0:  sb v0,0x5318(v1)
8010cfc4:  lui v1,0x8011
8010cfc8:  lw s0,0x50(sp)
8010cfcc:  addiu v1,v1,0x1ba0
8010cfd0:  sll v0,a0,0x2
8010cfd4:  addu v0,v0,a0
8010cfd8:  sll v0,v0,0x2
8010cfdc:  addu v0,v0,v1
8010cfe0:  lw v0,0x10(v0)
8010cfe4:  jr ra
8010cfe8:  _addiu sp,sp,0x78
