# addr: 0x8004dcbc  name: RCpolyF3A
8004dcbc:  lw t0,0x48(a3)
8004dcc0:  lw t1,0x4c(a3)
8004dcc4:  lw t2,0x50(a3)
8004dcc8:  gte_stH t9
8004dccc:  lw t4,0x14(t0)
8004dcd0:  lw t5,0x14(t1)
8004dcd4:  lw t6,0x14(t2)
8004dcd8:  sra t8,t9,0x1
8004dcdc:  sltu at,t4,t8
8004dce0:  beq at,zero,0x8004dd04
8004dce4:  _sltu at,t5,t8
8004dce8:  beq at,zero,0x8004dd04
8004dcec:  _sltu at,t6,t8
8004dcf0:  beq at,zero,0x8004dd04
8004dcf4:  _nop
8004dcf8:  move v0,a0
8004dcfc:  jr ra
8004dd00:  _nop
8004dd04:  gte_stOFX t9
8004dd08:  lw v0,0x4(a1)
8004dd0c:  lw v1,0x8(a1)
8004dd10:  sra t9,t9,0x10
8004dd14:  srl v0,v0,0x1
8004dd18:  srl v1,v1,0x1
8004dd1c:  addu t8,t9,v0
8004dd20:  lh t4,0x10(t0)
8004dd24:  lh t5,0x10(t1)
8004dd28:  lh t6,0x10(t2)
8004dd2c:  slt at,t8,t4
8004dd30:  beq at,zero,0x8004dd54
8004dd34:  _slt at,t8,t5
8004dd38:  beq at,zero,0x8004dd54
8004dd3c:  _slt at,t8,t6
8004dd40:  beq at,zero,0x8004dd54
8004dd44:  _nop
8004dd48:  move v0,a0
8004dd4c:  jr ra
8004dd50:  _nop
8004dd54:  subu t8,t9,v0
8004dd58:  slt at,t4,t8
8004dd5c:  beq at,zero,0x8004dd80
8004dd60:  _slt at,t5,t8
8004dd64:  beq at,zero,0x8004dd80
8004dd68:  _slt at,t6,t8
8004dd6c:  beq at,zero,0x8004dd80
8004dd70:  _nop
8004dd74:  move v0,a0
8004dd78:  jr ra
8004dd7c:  _nop
8004dd80:  gte_stOFY t9
8004dd84:  lh t4,0x12(t0)
8004dd88:  lh t5,0x12(t1)
8004dd8c:  lh t6,0x12(t2)
8004dd90:  sra t9,t9,0x10
8004dd94:  addu t8,t9,v1
8004dd98:  slt at,t8,t4
8004dd9c:  beq at,zero,0x8004ddc0
8004dda0:  _slt at,t8,t5
8004dda4:  beq at,zero,0x8004ddc0
8004dda8:  _slt at,t8,t6
8004ddac:  beq at,zero,0x8004ddc0
8004ddb0:  _nop
8004ddb4:  move v0,a0
8004ddb8:  jr ra
8004ddbc:  _nop
8004ddc0:  subu t8,t9,v1
8004ddc4:  slt at,t4,t8
8004ddc8:  beq at,zero,0x8004ddec
8004ddcc:  _slt at,t5,t8
8004ddd0:  beq at,zero,0x8004ddec
8004ddd4:  _slt at,t6,t8
8004ddd8:  beq at,zero,0x8004ddec
8004dddc:  _nop
8004dde0:  move v0,a0
8004dde4:  jr ra
8004dde8:  _nop
8004ddec:  lh t4,0x0(t0)
8004ddf0:  lh t5,0x0(t1)
8004ddf4:  lh t6,0x0(t2)
8004ddf8:  add t7,t4,t5
8004ddfc:  add t8,t5,t6
8004de00:  add t9,t6,t4
8004de04:  sra t7,t7,0x1
8004de08:  sra t8,t8,0x1
8004de0c:  sra t9,t9,0x1
8004de10:  sh t7,0x0(a3)
8004de14:  sh t8,0x18(a3)
8004de18:  sh t9,0x30(a3)
8004de1c:  lh t4,0x2(t0)
8004de20:  lh t5,0x2(t1)
8004de24:  lh t6,0x2(t2)
8004de28:  add t7,t4,t5
8004de2c:  add t8,t5,t6
8004de30:  add t9,t6,t4
8004de34:  sra t7,t7,0x1
8004de38:  sra t8,t8,0x1
8004de3c:  sra t9,t9,0x1
8004de40:  sh t7,0x2(a3)
8004de44:  sh t8,0x1a(a3)
8004de48:  sh t9,0x32(a3)
8004de4c:  lh t4,0x4(t0)
8004de50:  lh t5,0x4(t1)
8004de54:  lh t6,0x4(t2)
8004de58:  add t7,t4,t5
8004de5c:  add t8,t5,t6
8004de60:  add t9,t6,t4
8004de64:  sra t7,t7,0x1
8004de68:  sra t8,t8,0x1
8004de6c:  sra t9,t9,0x1
8004de70:  sh t7,0x4(a3)
8004de74:  sh t8,0x1c(a3)
8004de78:  sh t9,0x34(a3)
8004de7c:  ldv0 a3
8004de84:  gte_ldVXY1 0x18(a3)
8004de88:  gte_ldVZ1 0x1c(a3)
8004de8c:  gte_ldVXY2 0x30(a3)
8004de90:  gte_ldVZ2 0x34(a3)
8004de94:  nop
8004de98:  RTPT
8004de9c:  lw t4,0x0(a1)
8004dea0:  addiu a2,a2,0x1
8004dea4:  bne t4,a2,0x8004df1c
8004dea8:  _nop
8004deac:  move v1,ra
8004deb0:  lw t0,0x4c(a3)
8004deb4:  addiu t1,a3,0x18
8004deb8:  addiu t2,a3,0x0
8004debc:  gte_stSXY0 0x10(a3)
8004dec0:  gte_stSXY1 0x28(a3)
8004dec4:  gte_stSXY2 0x40(a3)
8004dec8:  jal 0x8004dfd4
8004decc:  _nop
8004ded0:  addiu t0,a3,0x0
8004ded4:  addiu t1,a3,0x18
8004ded8:  addiu t2,a3,0x30
8004dedc:  jal 0x8004dfd4
8004dee0:  _nop
8004dee4:  lw t0,0x48(a3)
8004dee8:  addiu t1,a3,0x0
8004deec:  addiu t2,a3,0x30
8004def0:  jal 0x8004dfd4
8004def4:  _nop
8004def8:  lw t0,0x50(a3)
8004defc:  addiu t1,a3,0x30
8004df00:  addiu t2,a3,0x18
8004df04:  jal 0x8004dfd4
8004df08:  _nop
8004df0c:  move ra,v1
8004df10:  addiu a2,a2,-0x1
8004df14:  b 0x8004dfc8
8004df18:  _nop
8004df1c:  addiu a3,a3,0x58
8004df20:  sw ra,0x54(a3)
8004df24:  lw t4,-0x10(a3)
8004df28:  addiu t5,a3,-0x58
8004df2c:  addiu t6,a3,-0x28
8004df30:  sw t4,0x48(a3)
8004df34:  sw t5,0x4c(a3)
8004df38:  sw t6,0x50(a3)
8004df3c:  gte_stSZ1 -0x44(a3)
8004df40:  gte_stSZ2 -0x2c(a3)
8004df44:  gte_stSZ3 -0x14(a3)
8004df48:  gte_stSXY0 -0x48(a3)
8004df4c:  gte_stSXY1 -0x30(a3)
8004df50:  gte_stSXY2 -0x18(a3)
8004df54:  jal 0x8004dcbc
8004df58:  _nop
8004df5c:  lw t4,-0xc(a3)
8004df60:  addiu t5,a3,-0x40
8004df64:  addiu t6,a3,-0x58
8004df68:  sw t4,0x48(a3)
8004df6c:  sw t5,0x4c(a3)
8004df70:  sw t6,0x50(a3)
8004df74:  jal 0x8004dcbc
8004df78:  _nop
8004df7c:  lw t4,-0x8(a3)
8004df80:  addiu t5,a3,-0x28
8004df84:  addiu t6,a3,-0x40
8004df88:  sw t4,0x48(a3)
8004df8c:  sw t5,0x4c(a3)
8004df90:  sw t6,0x50(a3)
8004df94:  jal 0x8004dcbc
8004df98:  _nop
8004df9c:  addiu t4,a3,-0x58
8004dfa0:  addiu t5,a3,-0x40
8004dfa4:  addiu t6,a3,-0x28
8004dfa8:  sw t4,0x48(a3)
8004dfac:  sw t5,0x4c(a3)
8004dfb0:  sw t6,0x50(a3)
8004dfb4:  jal 0x8004dcbc
8004dfb8:  _nop
8004dfbc:  lw ra,0x54(a3)
8004dfc0:  addiu a3,a3,-0x58
8004dfc4:  addiu a2,a2,-0x1
8004dfc8:  move v0,a0
8004dfcc:  jr ra
8004dfd0:  _nop
