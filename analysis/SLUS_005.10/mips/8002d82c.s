# addr: 0x8002d82c  name: FUN_8002d82c
8002d82c:  addiu sp,sp,-0x90
8002d830:  sw s7,0x84(sp)
8002d834:  move s7,a1
8002d838:  sw ra,0x8c(sp)
8002d83c:  sw s8,0x88(sp)
8002d840:  sw s6,0x80(sp)
8002d844:  sw s5,0x7c(sp)
8002d848:  sw s4,0x78(sp)
8002d84c:  sw s3,0x74(sp)
8002d850:  sw s2,0x70(sp)
8002d854:  sw s1,0x6c(sp)
8002d858:  sw s0,0x68(sp)
8002d85c:  lw v0,0x10(s7)
8002d860:  nop
8002d864:  lbu v1,0x4(v0)
8002d868:  lw s3,0x0(s7)
8002d86c:  li v0,0x3
8002d870:  beq v1,v0,0x8002e288
8002d874:  _move s5,a0
8002d878:  move a0,s5
8002d87c:  jal 0x8001f5a0
8002d880:  _move a1,s7
8002d884:  lbu v1,0x4(s3)
8002d888:  li v0,0x7
8002d88c:  bne v1,v0,0x8002da78
8002d890:  _li v0,0x1
8002d894:  lhu v0,0x11e(s5)
8002d898:  nop
8002d89c:  beq v0,zero,0x8002d8b4
8002d8a0:  _lui v1,0x80
8002d8a4:  jal 0x8002c018
8002d8a8:  _move a0,s5
8002d8ac:  j 0x8002e28c
8002d8b0:  _clear v0
8002d8b4:  lw v0,0x0(s3)
8002d8b8:  nop
8002d8bc:  and v0,v0,v1
8002d8c0:  beq v0,zero,0x8002da58
8002d8c4:  _nop
8002d8c8:  lw s2,0x80(s3)
8002d8cc:  nop
8002d8d0:  addiu s0,s2,0xa4
8002d8d4:  lbu v0,0x12(s0)
8002d8d8:  nop
8002d8dc:  beq v0,zero,0x8002d958
8002d8e0:  _nop
8002d8e4:  lh v1,0x6(s5)
8002d8e8:  lb v0,0x13(s0)
8002d8ec:  nop
8002d8f0:  bne v1,v0,0x8002d958
8002d8f4:  _nop
8002d8f8:  lhu v1,0xa(s3)
8002d8fc:  lbu v0,0x14(s0)
8002d900:  nop
8002d904:  beq v1,v0,0x8002d958
8002d908:  _nop
8002d90c:  lbu v0,0x15(s0)
8002d910:  nop
8002d914:  addiu v0,v0,0x1
8002d918:  sb v0,0x15(s0)
8002d91c:  andi v1,v0,0xff
8002d920:  li v0,0x1e
8002d924:  sb v0,0x12(s0)
8002d928:  lhu v0,0xc(s3)
8002d92c:  addiu v1,v1,0x1
8002d930:  mult v0,v1
8002d934:  ori v1,zero,0xffff
8002d938:  mflo a0
8002d93c:  slt v0,a0,v1
8002d940:  beq v0,zero,0x8002d94c
8002d944:  _nop
8002d948:  move v1,a0
8002d94c:  lbu v0,0xa(s3)
8002d950:  j 0x8002d980
8002d954:  _sh v1,0xc(s3)
8002d958:  lbu v0,0x15(s0)
8002d95c:  nop
8002d960:  beq v0,zero,0x8002d96c
8002d964:  _li v1,0x1e
8002d968:  li v1,0x1
8002d96c:  sb v1,0x12(s0)
8002d970:  lbu v0,0x6(s5)
8002d974:  nop
8002d978:  sb v0,0x13(s0)
8002d97c:  lbu v0,0xa(s3)
8002d980:  nop
8002d984:  sb v0,0x14(s0)
8002d988:  lw v1,0x0(s3)
8002d98c:  lui v0,0xff7f
8002d990:  ori v0,v0,0xffff
8002d994:  and v1,v1,v0
8002d998:  sw v1,0x0(s3)
8002d99c:  lhu v0,0xc(s5)
8002d9a0:  nop
8002d9a4:  bne v0,zero,0x8002da58
8002d9a8:  _lui v0,0x100
8002d9ac:  and v0,v1,v0
8002d9b0:  beq v0,zero,0x8002da58
8002d9b4:  _nop
8002d9b8:  lh v0,0x6(s2)
8002d9bc:  nop
8002d9c0:  bgez v0,0x8002da0c
8002d9c4:  _nop
8002d9c8:  jal 0x8002b940
8002d9cc:  _move a0,s5
8002d9d0:  addiu a0,sp,0x28
8002d9d4:  lui a1,0x8006
8002d9d8:  addiu a1,a1,0x572c
8002d9dc:  jal 0x80053004
8002d9e0:  _move a2,v0
8002d9e4:  lw v0,0x10(gp)
8002d9e8:  nop
8002d9ec:  beq v0,zero,0x8002da00
8002d9f0:  _nop
8002d9f4:  lh v0,0x6(s2)
8002d9f8:  j 0x8002da04
8002d9fc:  _subu a0,zero,v0
8002da00:  clear a0
8002da04:  jal 0x800129e8
8002da08:  _addiu a1,sp,0x28
8002da0c:  jal 0x8002bc18
8002da10:  _move a0,s5
8002da14:  addiu a0,s5,0x24
8002da18:  lui a1,0x800
8002da1c:  jal 0x8003fea8
8002da20:  _ori a1,a1,0xff
8002da24:  lw v1,0x84(s5)
8002da28:  lui a1,0xfff1
8002da2c:  ori a1,a1,0x1980
8002da30:  move a0,s2
8002da34:  ori v0,zero,0xc350
8002da38:  sw v0,0x94(s5)
8002da3c:  addu v1,v1,a1
8002da40:  jal 0x8002d44c
8002da44:  _sw v1,0x84(s5)
8002da48:  lbu v0,0x17(s0)
8002da4c:  nop
8002da50:  addiu v0,v0,0x1
8002da54:  sb v0,0x17(s0)
8002da58:  lhu a1,0xc(s3)
8002da5c:  move a0,s5
8002da60:  addiu a2,s7,0x14
8002da64:  li a3,0x1
8002da68:  jal 0x8002c6fc
8002da6c:  _subu a1,zero,a1
8002da70:  j 0x8002e28c
8002da74:  _clear v0
8002da78:  bne v1,v0,0x8002db34
8002da7c:  _li v0,0x2
8002da80:  lw v0,0x0(s3)
8002da84:  lui v1,0x1
8002da88:  and v0,v0,v1
8002da8c:  beq v0,zero,0x8002dab8
8002da90:  _nop
8002da94:  lw v0,0x54(s3)
8002da98:  nop
8002da9c:  bgez v0,0x8002daa8
8002daa0:  _nop
8002daa4:  addiu v0,v0,0xff
8002daa8:  lw v1,0x84(s3)
8002daac:  sra v0,v0,0x8
8002dab0:  j 0x8002dadc
8002dab4:  _mult v1,v0
8002dab8:  jal 0x80016a20
8002dabc:  _addiu a0,s3,0x88
8002dac0:  lw v1,0x54(s3)
8002dac4:  nop
8002dac8:  bgez v1,0x8002dad4
8002dacc:  _move a0,v0
8002dad0:  addiu v1,v1,0xff
8002dad4:  sra v0,v1,0x8
8002dad8:  mult a0,v0
8002dadc:  mflo v0
8002dae0:  bgez v0,0x8002daf0
8002dae4:  _sra v1,v0,0xc
8002dae8:  addiu v0,v0,0xfff
8002daec:  sra v1,v0,0xc
8002daf0:  lhu v0,0xc(s5)
8002daf4:  nop
8002daf8:  srl a1,v0,0x2
8002dafc:  slt v0,v1,a1
8002db00:  beq v0,zero,0x8002db0c
8002db04:  _nop
8002db08:  move a1,v1
8002db0c:  lw v0,0x0(s3)
8002db10:  move a0,s5
8002db14:  subu a1,zero,a1
8002db18:  addiu a2,s7,0x14
8002db1c:  clear a3
8002db20:  ori v0,v0,0x20
8002db24:  jal 0x8002c958
8002db28:  _sw v0,0x0(s3)
8002db2c:  j 0x8002e28c
8002db30:  _clear v0
8002db34:  bne v1,v0,0x8002dfc0
8002db38:  _addiu s0,s7,0x20
8002db3c:  lhu v0,0xa2(s5)
8002db40:  lw v1,0x80(s5)
8002db44:  srl v0,v0,0x6
8002db48:  mult v1,v0
8002db4c:  lhu v0,0xa2(s3)
8002db50:  lw v1,0x80(s3)
8002db54:  mflo a0
8002db58:  srl v0,v0,0x6
8002db5c:  nop
8002db60:  mult v1,v0
8002db64:  mflo t0
8002db68:  subu v0,a0,t0
8002db6c:  sw v0,0x28(sp)
8002db70:  lhu v0,0xa2(s5)
8002db74:  lw v1,0x84(s5)
8002db78:  srl v0,v0,0x6
8002db7c:  mult v1,v0
8002db80:  lhu v0,0xa2(s3)
8002db84:  lw v1,0x84(s3)
8002db88:  mflo a1
8002db8c:  srl v0,v0,0x6
8002db90:  nop
8002db94:  mult v1,v0
8002db98:  addiu a0,sp,0x28
8002db9c:  mflo t0
8002dba0:  subu v0,a1,t0
8002dba4:  sw v0,0x4(a0)
8002dba8:  lhu v0,0xa2(s5)
8002dbac:  lw v1,0x88(s5)
8002dbb0:  srl v0,v0,0x6
8002dbb4:  mult v1,v0
8002dbb8:  lhu v0,0xa2(s3)
8002dbbc:  lw v1,0x88(s3)
8002dbc0:  mflo a2
8002dbc4:  srl v0,v0,0x6
8002dbc8:  nop
8002dbcc:  mult v1,v0
8002dbd0:  move a1,s0
8002dbd4:  mflo t0
8002dbd8:  subu v0,a2,t0
8002dbdc:  jal 0x80017240
8002dbe0:  _sw v0,0x8(a0)
8002dbe4:  srl v0,v0,0xd
8002dbe8:  sll a0,v1,0x13
8002dbec:  or v0,v0,a0
8002dbf0:  sra v1,v1,0xd
8002dbf4:  move s4,v0
8002dbf8:  bgez s4,0x8002dfa4
8002dbfc:  _move s8,s3
8002dc00:  lhu v0,0xa2(s5)
8002dc04:  nop
8002dc08:  srl v0,v0,0x6
8002dc0c:  div s4,v0
8002dc10:  mflo s2
8002dc14:  addiu a0,s5,0x10
8002dc18:  move a1,s0
8002dc1c:  jal 0x800434d0
8002dc20:  _addiu a2,sp,0x18
8002dc24:  lw v1,0x18(sp)
8002dc28:  lw v0,0x30(s7)
8002dc2c:  move t4,v1
8002dc30:  sra t5,v1,0x1f
8002dc34:  sll v0,v0,0x1
8002dc38:  addu v0,v0,s2
8002dc3c:  subu v0,zero,v0
8002dc40:  move a2,v0
8002dc44:  sra a3,v0,0x1f
8002dc48:  multu t4,a2
8002dc4c:  addiu a1,sp,0x18
8002dc50:  lw v0,0x4(a1)
8002dc54:  mfhi s1
8002dc58:  mflo s0
8002dc5c:  move t0,v0
8002dc60:  sra t1,v0,0x1f
8002dc64:  multu t0,a2
8002dc68:  lw a0,0x8(a1)
8002dc6c:  mfhi t7
8002dc70:  mflo t6
8002dc74:  move v0,a0
8002dc78:  sra v1,a0,0x1f
8002dc7c:  multu v0,a2
8002dc80:  mfhi t3
8002dc84:  mflo t2
8002dc88:  nop
8002dc8c:  nop
8002dc90:  mult t4,a3
8002dc94:  mflo t9
8002dc98:  nop
8002dc9c:  nop
8002dca0:  mult a2,t5
8002dca4:  mflo t4
8002dca8:  nop
8002dcac:  nop
8002dcb0:  mult t0,a3
8002dcb4:  mflo t5
8002dcb8:  nop
8002dcbc:  nop
8002dcc0:  mult a2,t1
8002dcc4:  mflo t0
8002dcc8:  nop
8002dccc:  nop
8002dcd0:  mult v0,a3
8002dcd4:  mflo t1
8002dcd8:  nop
8002dcdc:  nop
8002dce0:  mult a2,v1
8002dce4:  addiu s6,s7,0x14
8002dce8:  move a0,s5
8002dcec:  addu s1,s1,t9
8002dcf0:  addu s1,s1,t4
8002dcf4:  addu t7,t7,t5
8002dcf8:  addu t7,t7,t0
8002dcfc:  move a2,s6
8002dd00:  addu t3,t3,t1
8002dd04:  mflo v0
8002dd08:  addu t3,t3,v0
8002dd0c:  srl s0,s0,0xc
8002dd10:  sll v0,s1,0x14
8002dd14:  or s0,s0,v0
8002dd18:  sra s1,s1,0xc
8002dd1c:  srl t6,t6,0xc
8002dd20:  sll v0,t7,0x14
8002dd24:  or t6,t6,v0
8002dd28:  sra t7,t7,0xc
8002dd2c:  srl t2,t2,0xc
8002dd30:  sll v0,t3,0x14
8002dd34:  or t2,t2,v0
8002dd38:  sra t3,t3,0xc
8002dd3c:  sw s0,0x18(sp)
8002dd40:  sw t6,0x4(a1)
8002dd44:  jal 0x80017594
8002dd48:  _sw t2,0x8(a1)
8002dd4c:  bgez s2,0x8002dd58
8002dd50:  _move v0,s2
8002dd54:  addiu v0,s2,0x1fff
8002dd58:  sra s2,v0,0xd
8002dd5c:  slti v0,s2,-0x8
8002dd60:  beq v0,zero,0x8002dda0
8002dd64:  _move a0,s5
8002dd68:  move a1,s2
8002dd6c:  move a2,s6
8002dd70:  jal 0x8002c958
8002dd74:  _li a3,0x1
8002dd78:  lh a0,0x6(s5)
8002dd7c:  nop
8002dd80:  bgez a0,0x8002dda0
8002dd84:  _nor a0,zero,a0
8002dd88:  li v0,0x40
8002dd8c:  sw v0,0x10(sp)
8002dd90:  li a1,0xa
8002dd94:  li a2,0xc0
8002dd98:  jal 0x80012028
8002dd9c:  _clear a3
8002dda0:  addiu a0,s5,0x10
8002dda4:  addiu s6,s7,0x14
8002dda8:  move a1,s6
8002ddac:  jal 0x80043408
8002ddb0:  _move a2,s6
8002ddb4:  addiu s0,s8,0x10
8002ddb8:  move a0,s0
8002ddbc:  move a1,s6
8002ddc0:  jal 0x800435c0
8002ddc4:  _move a2,s6
8002ddc8:  lhu v0,0xa2(s8)
8002ddcc:  nop
8002ddd0:  srl v0,v0,0x6
8002ddd4:  div s4,v0
8002ddd8:  mflo s2
8002dddc:  move a0,s0
8002dde0:  addiu a1,s7,0x20
8002dde4:  jal 0x800434d0
8002dde8:  _addiu a2,sp,0x18
8002ddec:  lw v1,0x18(sp)
8002ddf0:  lw v0,0x30(s7)
8002ddf4:  move t4,v1
8002ddf8:  sra t5,v1,0x1f
8002ddfc:  sll v0,v0,0x1
8002de00:  addu v0,v0,s2
8002de04:  move a2,v0
8002de08:  sra a3,v0,0x1f
8002de0c:  multu t4,a2
8002de10:  addiu a1,sp,0x18
8002de14:  lw v0,0x4(a1)
8002de18:  mfhi s1
8002de1c:  mflo s0
8002de20:  move t0,v0
8002de24:  sra t1,v0,0x1f
8002de28:  multu t0,a2
8002de2c:  lw a0,0x8(a1)
8002de30:  mfhi t7
8002de34:  mflo t6
8002de38:  move v0,a0
8002de3c:  sra v1,a0,0x1f
8002de40:  multu v0,a2
8002de44:  mfhi t3
8002de48:  mflo t2
8002de4c:  nop
8002de50:  nop
8002de54:  mult t4,a3
8002de58:  mflo s4
8002de5c:  nop
8002de60:  nop
8002de64:  mult a2,t5
8002de68:  mflo t4
8002de6c:  nop
8002de70:  nop
8002de74:  mult t0,a3
8002de78:  mflo t5
8002de7c:  nop
8002de80:  nop
8002de84:  mult a2,t1
8002de88:  mflo t0
8002de8c:  nop
8002de90:  nop
8002de94:  mult v0,a3
8002de98:  mflo t1
8002de9c:  nop
8002dea0:  nop
8002dea4:  mult a2,v1
8002dea8:  move a0,s8
8002deac:  addu s1,s1,s4
8002deb0:  addu s1,s1,t4
8002deb4:  addu t7,t7,t5
8002deb8:  addu t7,t7,t0
8002debc:  move a2,s6
8002dec0:  addu t3,t3,t1
8002dec4:  mflo v0
8002dec8:  addu t3,t3,v0
8002decc:  srl s0,s0,0xc
8002ded0:  sll v0,s1,0x14
8002ded4:  or s0,s0,v0
8002ded8:  sra s1,s1,0xc
8002dedc:  srl t6,t6,0xc
8002dee0:  sll v0,t7,0x14
8002dee4:  or t6,t6,v0
8002dee8:  sra t7,t7,0xc
8002deec:  srl t2,t2,0xc
8002def0:  sll v0,t3,0x14
8002def4:  or t2,t2,v0
8002def8:  sra t3,t3,0xc
8002defc:  sw s0,0x18(sp)
8002df00:  sw t6,0x4(a1)
8002df04:  jal 0x80017594
8002df08:  _sw t2,0x8(a1)
8002df0c:  bgez s2,0x8002df18
8002df10:  _move v0,s2
8002df14:  addiu v0,s2,0x1fff
8002df18:  sra s2,v0,0xd
8002df1c:  slti v0,s2,-0x8
8002df20:  beq v0,zero,0x8002df60
8002df24:  _move a0,s8
8002df28:  move a1,s2
8002df2c:  move a2,s6
8002df30:  jal 0x8002c958
8002df34:  _li a3,0x1
8002df38:  lh a0,0x6(s8)
8002df3c:  nop
8002df40:  bgez a0,0x8002df60
8002df44:  _li v0,0x40
8002df48:  sw v0,0x10(sp)
8002df4c:  nor a0,zero,a0
8002df50:  li a1,0xa
8002df54:  li a2,0xc0
8002df58:  jal 0x80012028
8002df5c:  _clear a3
8002df60:  lw v0,0x0(s5)
8002df64:  nop
8002df68:  andi v0,v0,0x8000
8002df6c:  bne v0,zero,0x8002dfa4
8002df70:  _nop
8002df74:  lw v0,0x8c(s5)
8002df78:  nop
8002df7c:  slti v0,v0,0x1ca
8002df80:  bne v0,zero,0x8002dfa4
8002df84:  _nop
8002df88:  jal 0x8004410c
8002df8c:  _nop
8002df90:  lw a1,0x5f8(gp)
8002df94:  move a0,v0
8002df98:  li a2,0x1e
8002df9c:  jal 0x8004483c
8002dfa0:  _addiu a3,s3,0x48
8002dfa4:  lw v1,0x0(s5)
8002dfa8:  lui a0,0x1
8002dfac:  ori a0,a0,0x8000
8002dfb0:  li v0,0x1
8002dfb4:  or v1,v1,a0
8002dfb8:  j 0x8002e28c
8002dfbc:  _sw v1,0x0(s5)
8002dfc0:  lw v0,0x74(s5)
8002dfc4:  nop
8002dfc8:  beq v0,s3,0x8002dfe0
8002dfcc:  _nop
8002dfd0:  lw v0,0x78(s5)
8002dfd4:  nop
8002dfd8:  bne v0,s3,0x8002dff8
8002dfdc:  _addiu a0,s5,0x80
8002dfe0:  lh v0,0x22(s7)
8002dfe4:  nop
8002dfe8:  slti v0,v0,-0x800
8002dfec:  bne v0,zero,0x8002e28c
8002dff0:  _clear v0
8002dff4:  addiu a0,s5,0x80
8002dff8:  jal 0x80017240
8002dffc:  _addiu a1,s7,0x20
8002e000:  srl v0,v0,0xd
8002e004:  sll a0,v1,0x13
8002e008:  or v0,v0,a0
8002e00c:  sra v1,v1,0xd
8002e010:  move s4,v0
8002e014:  bgez s4,0x8002e28c
8002e018:  _clear v0
8002e01c:  subu v0,zero,s4
8002e020:  bgez v0,0x8002e02c
8002e024:  _nop
8002e028:  addiu v0,v0,0x3fff
8002e02c:  lhu v1,0xa2(s5)
8002e030:  sra v0,v0,0xe
8002e034:  mult v0,v1
8002e038:  mflo v0
8002e03c:  bgez v0,0x8002e048
8002e040:  _nop
8002e044:  addiu v0,v0,0xfff
8002e048:  lw v1,0x64(s3)
8002e04c:  nop
8002e050:  beq v1,zero,0x8002e06c
8002e054:  _sra a2,v0,0xc
8002e058:  move a0,s3
8002e05c:  jalr v1
8002e060:  _li a1,0x8
8002e064:  j 0x8002e070
8002e068:  _nop
8002e06c:  clear v0
8002e070:  bne v0,zero,0x8002e28c
8002e074:  _clear v0
8002e078:  addiu a0,s5,0x10
8002e07c:  addiu a1,s7,0x20
8002e080:  jal 0x800434d0
8002e084:  _addiu a2,sp,0x18
8002e088:  lw v0,0x30(s7)
8002e08c:  lw v1,0x18(sp)
8002e090:  addu v0,v0,s4
8002e094:  subu v0,zero,v0
8002e098:  move a2,v0
8002e09c:  sra a3,v0,0x1f
8002e0a0:  move t4,v1
8002e0a4:  sra t5,v1,0x1f
8002e0a8:  multu t4,a2
8002e0ac:  addiu a1,sp,0x18
8002e0b0:  lw v0,0x4(a1)
8002e0b4:  mfhi s1
8002e0b8:  mflo s0
8002e0bc:  move t0,v0
8002e0c0:  sra t1,v0,0x1f
8002e0c4:  multu t0,a2
8002e0c8:  lw a0,0x8(a1)
8002e0cc:  mfhi t7
8002e0d0:  mflo t6
8002e0d4:  move v0,a0
8002e0d8:  sra v1,a0,0x1f
8002e0dc:  multu v0,a2
8002e0e0:  mfhi t3
8002e0e4:  mflo t2
8002e0e8:  nop
8002e0ec:  nop
8002e0f0:  mult t4,a3
8002e0f4:  mflo s6
8002e0f8:  nop
8002e0fc:  nop
8002e100:  mult a2,t5
8002e104:  mflo t4
8002e108:  nop
8002e10c:  nop
8002e110:  mult t0,a3
8002e114:  mflo t5
8002e118:  nop
8002e11c:  nop
8002e120:  mult a2,t1
8002e124:  mflo t0
8002e128:  nop
8002e12c:  nop
8002e130:  mult v0,a3
8002e134:  mflo t1
8002e138:  nop
8002e13c:  nop
8002e140:  mult a2,v1
8002e144:  addiu s2,s7,0x14
8002e148:  move a0,s5
8002e14c:  addu s1,s1,s6
8002e150:  addu s1,s1,t4
8002e154:  addu t7,t7,t5
8002e158:  addu t7,t7,t0
8002e15c:  move a2,s2
8002e160:  addu t3,t3,t1
8002e164:  mflo v0
8002e168:  addu t3,t3,v0
8002e16c:  srl s0,s0,0xc
8002e170:  sll v0,s1,0x14
8002e174:  or s0,s0,v0
8002e178:  sra s1,s1,0xc
8002e17c:  srl t6,t6,0xc
8002e180:  sll v0,t7,0x14
8002e184:  or t6,t6,v0
8002e188:  sra t7,t7,0xc
8002e18c:  srl t2,t2,0xc
8002e190:  sll v0,t3,0x14
8002e194:  or t2,t2,v0
8002e198:  sra t3,t3,0xc
8002e19c:  sw s0,0x18(sp)
8002e1a0:  sw t6,0x4(a1)
8002e1a4:  jal 0x80017594
8002e1a8:  _sw t2,0x8(a1)
8002e1ac:  lw v0,0x0(s5)
8002e1b0:  nop
8002e1b4:  andi v0,v0,0x8000
8002e1b8:  bne v0,zero,0x8002e274
8002e1bc:  _move v1,s4
8002e1c0:  bgez v1,0x8002e1cc
8002e1c4:  _nop
8002e1c8:  addiu v1,v1,0x3fff
8002e1cc:  lhu v0,0xc(s3)
8002e1d0:  sra v1,v1,0xe
8002e1d4:  subu a1,zero,v0
8002e1d8:  slt v0,a1,v1
8002e1dc:  beq v0,zero,0x8002e1e8
8002e1e0:  _move a0,s5
8002e1e4:  move a1,v1
8002e1e8:  move a2,s2
8002e1ec:  jal 0x8002c958
8002e1f0:  _clear a3
8002e1f4:  lh a0,0x6(s5)
8002e1f8:  nop
8002e1fc:  bgez a0,0x8002e214
8002e200:  _nor a0,zero,a0
8002e204:  li a1,0xff
8002e208:  clear a2
8002e20c:  jal 0x80012068
8002e210:  _li a3,0x40
8002e214:  lw v0,0x8c(s5)
8002e218:  nop
8002e21c:  slti v0,v0,0x1ca
8002e220:  bne v0,zero,0x8002e274
8002e224:  _li a1,0xb
8002e228:  lhu v0,0x6(s3)
8002e22c:  lui v1,0x8001
8002e230:  srl a0,v0,0x4
8002e234:  slt v0,a0,a1
8002e238:  beq v0,zero,0x8002e244
8002e23c:  _addiu v1,v1,0x57c
8002e240:  move a1,a0
8002e244:  addu v0,a1,v1
8002e248:  lbu s0,0x0(v0)
8002e24c:  li v0,0xff
8002e250:  beq s0,v0,0x8002e274
8002e254:  _nop
8002e258:  jal 0x8004410c
8002e25c:  _nop
8002e260:  lw a1,0x5f8(gp)
8002e264:  move a0,v0
8002e268:  move a2,s0
8002e26c:  jal 0x8004483c
8002e270:  _addiu a3,s3,0x24
8002e274:  lw v0,0x0(s5)
8002e278:  lui v1,0x1
8002e27c:  ori v1,v1,0x8000
8002e280:  or v0,v0,v1
8002e284:  sw v0,0x0(s5)
8002e288:  clear v0
8002e28c:  lw ra,0x8c(sp)
8002e290:  lw s8,0x88(sp)
8002e294:  lw s7,0x84(sp)
8002e298:  lw s6,0x80(sp)
8002e29c:  lw s5,0x7c(sp)
8002e2a0:  lw s4,0x78(sp)
8002e2a4:  lw s3,0x74(sp)
8002e2a8:  lw s2,0x70(sp)
8002e2ac:  lw s1,0x6c(sp)
8002e2b0:  lw s0,0x68(sp)
8002e2b4:  jr ra
8002e2b8:  _addiu sp,sp,0x90
