# addr: 0x8001de08  name: FUN_8001de08
8001de08:  addiu sp,sp,-0x60
8001de0c:  sw s0,0x50(sp)
8001de10:  move s0,a0
8001de14:  sw ra,0x58(sp)
8001de18:  sw s1,0x54(sp)
8001de1c:  lw v0,0x0(s0)
8001de20:  nop
8001de24:  andi v0,v0,0x2
8001de28:  bne v0,zero,0x8001e10c
8001de2c:  _nop
8001de30:  lw a1,0x54(s0)
8001de34:  jal 0x8001db54
8001de38:  _addiu a0,s0,0x24
8001de3c:  beq v0,zero,0x8001e10c
8001de40:  _addiu s1,sp,0x10
8001de44:  lui a0,0x8007
8001de48:  addiu a0,a0,-0x980
8001de4c:  addiu a1,s0,0x10
8001de50:  jal 0x8004cf04
8001de54:  _move a2,s1
8001de58:  lw v1,0x2c(sp)
8001de5c:  lui v0,0x3f
8001de60:  ori v0,v0,0xffff
8001de64:  slt v0,v0,v1
8001de68:  bne v0,zero,0x8001e10c
8001de6c:  _nop
8001de70:  lw v1,0x0(s0)
8001de74:  nop
8001de78:  andi v0,v1,0x10
8001de7c:  beq v0,zero,0x8001df24
8001de80:  _andi v0,v1,0x400
8001de84:  beq v0,zero,0x8001ded8
8001de88:  _nop
8001de8c:  lw a3,0x10(s0)
8001de90:  lw t0,0x14(s0)
8001de94:  lw t1,0x18(s0)
8001de98:  lw t2,0x1c(s0)
8001de9c:  sw a3,0x30(sp)
8001dea0:  sw t0,0x34(sp)
8001dea4:  sw t1,0x38(sp)
8001dea8:  sw t2,0x3c(sp)
8001deac:  lh a3,0x20(s0)
8001deb0:  nop
8001deb4:  sh a3,0x40(sp)
8001deb8:  lw a3,0x24(sp)
8001debc:  lw t0,0x28(sp)
8001dec0:  lw t1,0x2c(sp)
8001dec4:  sw a3,0x44(sp)
8001dec8:  sw t0,0x48(sp)
8001decc:  sw t1,0x4c(sp)
8001ded0:  j 0x8001df24
8001ded4:  _addiu s1,sp,0x30
8001ded8:  lh v0,0x22(s0)
8001dedc:  nop
8001dee0:  beq v0,zero,0x8001def8
8001dee4:  _lui v0,0x8007
8001dee8:  jal 0x80016e64
8001deec:  _move a0,s1
8001def0:  j 0x8001df24
8001def4:  _nop
8001def8:  addiu t2,v0,-0x9a0
8001defc:  lw a3,0x0(t2)
8001df00:  lw t0,0x4(t2)
8001df04:  lw t1,0x8(t2)
8001df08:  sw a3,0x10(sp)
8001df0c:  sw t0,0x14(sp)
8001df10:  sw t1,0x18(sp)
8001df14:  lw a3,0xc(t2)
8001df18:  lh t0,0x10(t2)
8001df1c:  sw a3,0x1c(sp)
8001df20:  sh t0,0x20(sp)
8001df24:  lw v0,0x0(s0)
8001df28:  nop
8001df2c:  andi v0,v0,0x2000
8001df30:  beq v0,zero,0x8001dfb4
8001df34:  _li a0,0x40
8001df38:  lw v1,0x48(s0)
8001df3c:  nop
8001df40:  bgez v1,0x8001df4c
8001df44:  _ori v0,zero,0xffff
8001df48:  addu v1,v1,v0
8001df4c:  lw a0,0x50(s0)
8001df50:  nop
8001df54:  bgez a0,0x8001df64
8001df58:  _sra a2,v1,0x10
8001df5c:  ori v0,zero,0xffff
8001df60:  addu a0,a0,v0
8001df64:  sra a0,a0,0x10
8001df68:  lui a1,0x8009
8001df6c:  addiu a1,a1,0x1120
8001df70:  srl v0,a0,0x6
8001df74:  sll v0,v0,0x2
8001df78:  srl v1,a2,0x6
8001df7c:  sll v1,v1,0x7
8001df80:  addu v0,v0,v1
8001df84:  addu v0,v0,a1
8001df88:  lw v1,0x80(v0)
8001df8c:  andi a0,a0,0x3f
8001df90:  sll a0,a0,0x1
8001df94:  andi v0,a2,0x3f
8001df98:  sll v0,v0,0x7
8001df9c:  addu a0,a0,v0
8001dfa0:  addu v1,v1,a0
8001dfa4:  lhu a0,0x0(v1)
8001dfa8:  nop
8001dfac:  andi a0,a0,0xf800
8001dfb0:  srl a0,a0,0x8
8001dfb4:  move a1,a0
8001dfb8:  jal 0x8004d4e4
8001dfbc:  _move a2,a0
8001dfc0:  lw v1,0x6c(s0)
8001dfc4:  nop
8001dfc8:  beq v1,zero,0x8001e0a8
8001dfcc:  _nop
8001dfd0:  lw v0,0x2c(sp)
8001dfd4:  nop
8001dfd8:  slt v0,v1,v0
8001dfdc:  beq v0,zero,0x8001e0a8
8001dfe0:  _nop
8001dfe4:  lw v0,0x68(s0)
8001dfe8:  nop
8001dfec:  beq v0,zero,0x8001e0dc
8001dff0:  _li v0,0x1000
8001dff4:  lw a0,0x0(s0)
8001dff8:  nop
8001dffc:  andi v1,a0,0x1010
8001e000:  bne v1,v0,0x8001e090
8001e004:  _andi v0,a0,0x400
8001e008:  beq v0,zero,0x8001e044
8001e00c:  _lui v0,0x8005
8001e010:  addiu t2,v0,0x68b4
8001e014:  lw a3,0x0(t2)
8001e018:  lw t0,0x4(t2)
8001e01c:  lw t1,0x8(t2)
8001e020:  sw a3,0x10(sp)
8001e024:  sw t0,0x14(sp)
8001e028:  sw t1,0x18(sp)
8001e02c:  lw a3,0xc(t2)
8001e030:  lh t0,0x10(t2)
8001e034:  sw a3,0x1c(sp)
8001e038:  sh t0,0x20(sp)
8001e03c:  j 0x8001e090
8001e040:  _nop
8001e044:  lh v0,0x22(s0)
8001e048:  nop
8001e04c:  beq v0,zero,0x8001e064
8001e050:  _lui v0,0x8007
8001e054:  jal 0x80016e64
8001e058:  _addiu a0,sp,0x10
8001e05c:  j 0x8001e090
8001e060:  _nop
8001e064:  addiu t2,v0,-0x9a0
8001e068:  lw a3,0x0(t2)
8001e06c:  lw t0,0x4(t2)
8001e070:  lw t1,0x8(t2)
8001e074:  sw a3,0x10(sp)
8001e078:  sw t0,0x14(sp)
8001e07c:  sw t1,0x18(sp)
8001e080:  lw a3,0xc(t2)
8001e084:  lh t0,0x10(t2)
8001e088:  sw a3,0x1c(sp)
8001e08c:  sh t0,0x20(sp)
8001e090:  lw a0,0x68(s0)
8001e094:  lw a2,0x60c(gp)
8001e098:  jal 0x8001be5c
8001e09c:  _move a1,s1
8001e0a0:  j 0x8001e0dc
8001e0a4:  _nop
8001e0a8:  lw a0,0x30(s0)
8001e0ac:  nop
8001e0b0:  beq a0,zero,0x8001e0c4
8001e0b4:  _nop
8001e0b8:  lw a2,0x60c(gp)
8001e0bc:  jal 0x8001be5c
8001e0c0:  _move a1,s1
8001e0c4:  lw a0,0x38(s0)
8001e0c8:  nop
8001e0cc:  beq a0,zero,0x8001e0dc
8001e0d0:  _nop
8001e0d4:  jal 0x8001dcc8
8001e0d8:  _addiu a1,sp,0x10
8001e0dc:  lw v1,0x0(s0)
8001e0e0:  nop
8001e0e4:  andi v0,v1,0x8
8001e0e8:  beq v0,zero,0x8001e10c
8001e0ec:  _andi v0,v1,0x200
8001e0f0:  bne v0,zero,0x8001e100
8001e0f4:  _nop
8001e0f8:  jal 0x8003e2fc
8001e0fc:  _move a0,s0
8001e100:  lw a0,0x70(s0)
8001e104:  jal 0x8003e520
8001e108:  _nop
8001e10c:  lw ra,0x58(sp)
8001e110:  lw s1,0x54(sp)
8001e114:  lw s0,0x50(sp)
8001e118:  jr ra
8001e11c:  _addiu sp,sp,0x60
