# addr: 0x8001b0c4  name: FUN_8001b0c4
8001b0c4:  addiu sp,sp,-0x20
8001b0c8:  sw s1,0x14(sp)
8001b0cc:  move s1,a0
8001b0d0:  sw ra,0x18(sp)
8001b0d4:  sw s0,0x10(sp)
8001b0d8:  lw a0,0x58(s1)
8001b0dc:  nop
8001b0e0:  beq a0,zero,0x8001b1dc
8001b0e4:  _clear v0
8001b0e8:  lhu v0,0xa(s1)
8001b0ec:  lw a1,0x0(a0)
8001b0f0:  sll v1,v0,0x3
8001b0f4:  subu v1,v1,v0
8001b0f8:  sll v1,v1,0x2
8001b0fc:  addu v1,a1,v1
8001b100:  lhu v1,0x36(v1)
8001b104:  ori v0,zero,0xffff
8001b108:  beq v1,v0,0x8001b1dc
8001b10c:  _clear v0
8001b110:  sll v0,v1,0x3
8001b114:  subu v0,v0,v1
8001b118:  sll a3,v0,0x2
8001b11c:  addu a2,a1,a3
8001b120:  lhu v0,0x1c(a2)
8001b124:  ori v1,zero,0xc000
8001b128:  andi v0,v0,0xf000
8001b12c:  bne v0,v1,0x8001b1c8
8001b130:  _ori v0,zero,0xffff
8001b134:  addiu v0,a3,0x1c
8001b138:  addu s0,a1,v0
8001b13c:  lhu v0,0x0(s0)
8001b140:  nop
8001b144:  andi v0,v0,0x800
8001b148:  beq v0,zero,0x8001b160
8001b14c:  _nop
8001b150:  lw v0,0x0(s1)
8001b154:  nop
8001b158:  ori v0,v0,0x1000
8001b15c:  sw v0,0x0(s1)
8001b160:  lhu a1,0x0(s0)
8001b164:  jal 0x8001b49c
8001b168:  _andi a1,a1,0x7ff
8001b16c:  sw v0,0x68(s1)
8001b170:  lhu v0,0x16(s0)
8001b174:  lbu a0,0x6cf(gp)
8001b178:  beq v0,zero,0x8001b188
8001b17c:  _sll v0,v0,0x10
8001b180:  j 0x8001b1a8
8001b184:  _mult v0,a0
8001b188:  lh v1,0x5e4(gp)
8001b18c:  lw v0,0x54(s1)
8001b190:  nop
8001b194:  mult v0,v1
8001b198:  mflo v1
8001b19c:  nop
8001b1a0:  nop
8001b1a4:  mult v1,a0
8001b1a8:  mflo v0
8001b1ac:  bgez v0,0x8001b1b8
8001b1b0:  _nop
8001b1b4:  addiu v0,v0,0xff
8001b1b8:  sra v0,v0,0x8
8001b1bc:  sw v0,0x6c(s1)
8001b1c0:  j 0x8001b1e4
8001b1c4:  _li v0,0x1
8001b1c8:  lhu v1,0x34(a2)
8001b1cc:  nop
8001b1d0:  bne v1,v0,0x8001b114
8001b1d4:  _sll v0,v1,0x3
8001b1d8:  clear v0
8001b1dc:  sw zero,0x68(s1)
8001b1e0:  sw zero,0x6c(s1)
8001b1e4:  lw ra,0x18(sp)
8001b1e8:  lw s1,0x14(sp)
8001b1ec:  lw s0,0x10(sp)
8001b1f0:  jr ra
8001b1f4:  _addiu sp,sp,0x20
