# addr: 0x8002c018  name: FUN_8002c018
8002c018:  addiu sp,sp,-0x28
8002c01c:  sw s3,0x1c(sp)
8002c020:  move s3,a0
8002c024:  sw ra,0x24(sp)
8002c028:  sw s4,0x20(sp)
8002c02c:  sw s2,0x18(sp)
8002c030:  sw s1,0x14(sp)
8002c034:  sw s0,0x10(sp)
8002c038:  lw v0,0x0(s3)
8002c03c:  lui s4,0x2
8002c040:  and v0,v0,s4
8002c044:  beq v0,zero,0x8002c0b0
8002c048:  _lui v0,0x8007
8002c04c:  lw s1,0x38(s3)
8002c050:  nop
8002c054:  beq s1,zero,0x8002c1f0
8002c058:  _li a1,0x26
8002c05c:  lui v0,0x8007
8002c060:  addiu a0,v0,0x37a0
8002c064:  lhu v0,0xa(s1)
8002c068:  nop
8002c06c:  bne v0,a1,0x8002c088
8002c070:  _nop
8002c074:  lw v1,0x58(s1)
8002c078:  lw v0,0x3c(a0)
8002c07c:  nop
8002c080:  beq v1,v0,0x8002c098
8002c084:  _nop
8002c088:  lw s1,0x34(s1)
8002c08c:  nop
8002c090:  bne s1,zero,0x8002c064
8002c094:  _nop
8002c098:  beq s1,zero,0x8002c1f0
8002c09c:  _nop
8002c0a0:  jal 0x8001ac08
8002c0a4:  _move a0,s1
8002c0a8:  j 0x8002c1f0
8002c0ac:  _nop
8002c0b0:  lw a0,0x37dc(v0)
8002c0b4:  li a1,0x26
8002c0b8:  li a2,0x80
8002c0bc:  lw s0,0x5c(s3)
8002c0c0:  jal 0x8001ac44
8002c0c4:  _li a3,0x8
8002c0c8:  move s1,v0
8002c0cc:  lui v0,0x8003
8002c0d0:  addiu v0,v0,-0x4048
8002c0d4:  sw v0,0x64(s1)
8002c0d8:  lw v0,0x4(s0)
8002c0dc:  addiu s2,s0,0x4
8002c0e0:  lw v1,0xc(s2)
8002c0e4:  nop
8002c0e8:  addu v0,v0,v1
8002c0ec:  srl v1,v0,0x1f
8002c0f0:  addu v0,v0,v1
8002c0f4:  sra v0,v0,0x1
8002c0f8:  sw v0,0x24(s1)
8002c0fc:  lw v0,0x10(s2)
8002c100:  nop
8002c104:  sw v0,0x28(s1)
8002c108:  lw v0,0x8(s2)
8002c10c:  lw v1,0x14(s2)
8002c110:  sh zero,0x1e(s1)
8002c114:  sh zero,0x1c(s1)
8002c118:  sh zero,0x1a(s1)
8002c11c:  sh zero,0x16(s1)
8002c120:  sh zero,0x14(s1)
8002c124:  sh zero,0x12(s1)
8002c128:  addu v0,v0,v1
8002c12c:  srl v1,v0,0x1f
8002c130:  addu v0,v0,v1
8002c134:  sra v0,v0,0x1
8002c138:  sw v0,0x2c(s1)
8002c13c:  lw v1,0xc(s2)
8002c140:  lw v0,0x4(s0)
8002c144:  nop
8002c148:  subu v0,v1,v0
8002c14c:  bgez v0,0x8002c158
8002c150:  _nop
8002c154:  addiu v0,v0,0x7
8002c158:  sra v0,v0,0x3
8002c15c:  sh v0,0x10(s1)
8002c160:  lw v1,0x10(s2)
8002c164:  lw v0,0x4(s2)
8002c168:  nop
8002c16c:  subu v0,v1,v0
8002c170:  bgez v0,0x8002c17c
8002c174:  _nop
8002c178:  addiu v0,v0,0x3
8002c17c:  sra v0,v0,0x2
8002c180:  sh v0,0x18(s1)
8002c184:  lw v1,0x14(s2)
8002c188:  lw v0,0x8(s2)
8002c18c:  nop
8002c190:  subu v0,v1,v0
8002c194:  bgez v0,0x8002c1a0
8002c198:  _move a0,s3
8002c19c:  addiu v0,v0,0x7
8002c1a0:  move a1,s1
8002c1a4:  sra v0,v0,0x3
8002c1a8:  jal 0x8001d544
8002c1ac:  _sh v0,0x20(s1)
8002c1b0:  lw v0,0x0(s3)
8002c1b4:  nop
8002c1b8:  or v0,v0,s4
8002c1bc:  sw v0,0x0(s3)
8002c1c0:  andi v0,v0,0x4
8002c1c4:  bne v0,zero,0x8002c1d4
8002c1c8:  _nop
8002c1cc:  jal 0x800207c4
8002c1d0:  _move a0,s1
8002c1d4:  jal 0x8004410c
8002c1d8:  _nop
8002c1dc:  lw a1,0x5f8(gp)
8002c1e0:  move a0,v0
8002c1e4:  li a2,0x2d
8002c1e8:  jal 0x8004483c
8002c1ec:  _addiu a3,s3,0x24
8002c1f0:  lw ra,0x24(sp)
8002c1f4:  lw s4,0x20(sp)
8002c1f8:  lw s3,0x1c(sp)
8002c1fc:  lw s2,0x18(sp)
8002c200:  lw s1,0x14(sp)
8002c204:  lw s0,0x10(sp)
8002c208:  jr ra
8002c20c:  _addiu sp,sp,0x28
