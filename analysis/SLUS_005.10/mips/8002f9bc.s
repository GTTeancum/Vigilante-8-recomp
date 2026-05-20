# addr: 0x8002f9bc  name: FUN_8002f9bc
8002f9bc:  addiu sp,sp,-0x118
8002f9c0:  sw s6,0x108(sp)
8002f9c4:  move s6,a0
8002f9c8:  sw ra,0x114(sp)
8002f9cc:  sw s8,0x110(sp)
8002f9d0:  sw s7,0x10c(sp)
8002f9d4:  sw s5,0x104(sp)
8002f9d8:  sw s4,0x100(sp)
8002f9dc:  sw s3,0xfc(sp)
8002f9e0:  sw s2,0xf8(sp)
8002f9e4:  sw s1,0xf4(sp)
8002f9e8:  sw s0,0xf0(sp)
8002f9ec:  lw v0,0x0(s6)
8002f9f0:  lui v1,0xff8f
8002f9f4:  ori v1,v1,0xffff
8002f9f8:  addiu s1,sp,0x20
8002f9fc:  and v0,v0,v1
8002fa00:  sw v0,0x0(s6)
8002fa04:  addiu v0,sp,0x10
8002fa08:  sw zero,0x10(sp)
8002fa0c:  sw zero,0x4(v0)
8002fa10:  sw zero,0x8(v0)
8002fa14:  sw zero,0x20(sp)
8002fa18:  sw zero,0x4(s1)
8002fa1c:  sw zero,0x8(s1)
8002fa20:  lw a1,0xfc(s6)
8002fa24:  lw v1,0x100(s6)
8002fa28:  lhu v0,0xa4(s6)
8002fa2c:  addiu s0,s6,0x80
8002fa30:  move a0,s0
8002fa34:  addiu t6,s6,0xa4
8002fa38:  sh v0,0x42(v1)
8002fa3c:  sh v0,0x42(a1)
8002fa40:  jal 0x80016a20
8002fa44:  _sw t6,0xbc(sp)
8002fa48:  bgez v0,0x8002fa54
8002fa4c:  _nop
8002fa50:  addiu v0,v0,0x7f
8002fa54:  lh v1,0x18(s6)
8002fa58:  sra v0,v0,0x7
8002fa5c:  blez v1,0x80030434
8002fa60:  _sw v0,0x8c(s6)
8002fa64:  addiu a0,s6,0x10
8002fa68:  move a1,s0
8002fa6c:  jal 0x8004352c
8002fa70:  _addiu a2,sp,0x30
8002fa74:  clear s5
8002fa78:  addiu s3,sp,0x40
8002fa7c:  lui s7,0x1f80
8002fa80:  addiu s7,s7,0x0
8002fa84:  addiu s1,s7,0xc
8002fa88:  lui s0,0x1f80
8002fa8c:  addiu s0,s0,0x0
8002fa90:  move s4,s5
8002fa94:  li s2,0x10
8002fa98:  addu v0,s6,s2
8002fa9c:  lw a0,0xec(v0)
8002faa0:  nop
8002faa4:  lw v0,0x48(a0)
8002faa8:  nop
8002faac:  sw v0,0x40(sp)
8002fab0:  lw v0,0x4c(a0)
8002fab4:  lw v1,0x90(a0)
8002fab8:  nop
8002fabc:  addu v0,v0,v1
8002fac0:  sw v0,0x4(s3)
8002fac4:  lw v0,0x50(a0)
8002fac8:  move a1,s3
8002facc:  move a2,s0
8002fad0:  addiu s2,s2,0x4
8002fad4:  addiu s5,s5,0x1
8002fad8:  addiu a0,s6,0x10
8002fadc:  jal 0x80043408
8002fae0:  _sw v0,0x8(s3)
8002fae4:  move a0,s6
8002fae8:  move a1,s0
8002faec:  lui t8,0x1f80
8002faf0:  addiu t8,t8,0x0
8002faf4:  addiu a2,t8,0x10
8002faf8:  addu a2,s4,a2
8002fafc:  jal 0x8001d748
8002fb00:  _move a3,s1
8002fb04:  addiu s1,s1,0x18
8002fb08:  sw v0,0x4(s0)
8002fb0c:  addiu s0,s0,0x18
8002fb10:  slti v0,s5,0x4
8002fb14:  bne v0,zero,0x8002fa98
8002fb18:  _addiu s4,s4,0x18
8002fb1c:  clear s5
8002fb20:  addiu t9,sp,0x90
8002fb24:  addiu t6,sp,0xa0
8002fb28:  addiu s7,s6,0x80
8002fb2c:  sw t9,0xc4(sp)
8002fb30:  sw t6,0xc8(sp)
8002fb34:  sw s7,0xcc(sp)
8002fb38:  addiu v0,s5,0x4
8002fb3c:  sll v0,v0,0x2
8002fb40:  addu v0,s6,v0
8002fb44:  lw s4,0xec(v0)
8002fb48:  nop
8002fb4c:  lhu v0,0x42(s4)
8002fb50:  lui v1,0x8006
8002fb54:  addiu v1,v1,0x7b4
8002fb58:  andi v0,v0,0xfff
8002fb5c:  sll v0,v0,0x2
8002fb60:  addu v0,v0,v1
8002fb64:  lh s8,0x0(v0)
8002fb68:  lh v0,0x2(v0)
8002fb6c:  lw v1,0x48(s4)
8002fb70:  sw v0,0xc0(sp)
8002fb74:  sw v1,0x60(sp)
8002fb78:  lw v0,0x4c(s4)
8002fb7c:  lw v1,0x90(s4)
8002fb80:  addiu a0,sp,0x60
8002fb84:  addu v0,v0,v1
8002fb88:  sw v0,0x4(a0)
8002fb8c:  lw v0,0x50(s4)
8002fb90:  nop
8002fb94:  sw v0,0x8(a0)
8002fb98:  lw v1,0x94(s6)
8002fb9c:  lw v0,0x50(s4)
8002fba0:  nop
8002fba4:  mult v1,v0
8002fba8:  mflo v0
8002fbac:  bgez v0,0x8002fbb8
8002fbb0:  _nop
8002fbb4:  addiu v0,v0,0xfff
8002fbb8:  lw v1,0x30(sp)
8002fbbc:  lw a0,0x34(sp)
8002fbc0:  lw t6,0xc4(sp)
8002fbc4:  sra v0,v0,0xc
8002fbc8:  addu v1,v1,v0
8002fbcc:  sw v1,0x90(sp)
8002fbd0:  sw a0,0x4(t6)
8002fbd4:  lw v1,0x94(s6)
8002fbd8:  lw v0,0x48(s4)
8002fbdc:  nop
8002fbe0:  mult v1,v0
8002fbe4:  mflo v1
8002fbe8:  bgez v1,0x8002fbf4
8002fbec:  _nop
8002fbf0:  addiu v1,v1,0xfff
8002fbf4:  lw v0,0x38(sp)
8002fbf8:  lw t6,0xc4(sp)
8002fbfc:  sra v1,v1,0xc
8002fc00:  subu v0,v0,v1
8002fc04:  lui v1,0x1f80
8002fc08:  addiu v1,v1,0x0
8002fc0c:  sw v0,0x8(t6)
8002fc10:  sll v0,s5,0x1
8002fc14:  addu v0,v0,s5
8002fc18:  sll v0,v0,0x3
8002fc1c:  addu v0,v0,v1
8002fc20:  lw s7,0x0(v0)
8002fc24:  lw t8,0x4(v0)
8002fc28:  lw t9,0x8(v0)
8002fc2c:  sw s7,0x70(sp)
8002fc30:  sw t8,0x74(sp)
8002fc34:  sw t9,0x78(sp)
8002fc38:  lw s7,0x10(v0)
8002fc3c:  lw t8,0x14(v0)
8002fc40:  sw s7,0xa0(sp)
8002fc44:  sw t8,0xa4(sp)
8002fc48:  lw v0,0xc(v0)
8002fc4c:  addiu a0,s6,0x10
8002fc50:  addiu a1,sp,0x70
8002fc54:  addiu a2,sp,0x80
8002fc58:  jal 0x800435c0
8002fc5c:  _sw v0,0xd0(sp)
8002fc60:  lw v1,0x84(sp)
8002fc64:  lw v0,0x90(s4)
8002fc68:  nop
8002fc6c:  subu v1,v1,v0
8002fc70:  sw v1,0x84(sp)
8002fc74:  lw v0,0x84(s4)
8002fc78:  nop
8002fc7c:  slt v1,v1,v0
8002fc80:  beq v1,zero,0x80030378
8002fc84:  _nop
8002fc88:  lw s7,0xd0(sp)
8002fc8c:  lw v1,0x0(s6)
8002fc90:  beq s7,zero,0x8002fc9c
8002fc94:  _lui v0,0x10
8002fc98:  lui v0,0x30
8002fc9c:  or v0,v1,v0
8002fca0:  sw v0,0x0(s6)
8002fca4:  lhu v0,0xa0(sp)
8002fca8:  lw v1,0x80(s6)
8002fcac:  lw t8,0xc8(sp)
8002fcb0:  lw t6,0xcc(sp)
8002fcb4:  sll v0,v0,0x10
8002fcb8:  sra s2,v0,0x10
8002fcbc:  sra s3,v0,0x1f
8002fcc0:  move s0,v1
8002fcc4:  sra s1,v1,0x1f
8002fcc8:  multu s2,s0
8002fccc:  lhu v0,0x2(t8)
8002fcd0:  lw v1,0x4(t6)
8002fcd4:  sll v0,v0,0x10
8002fcd8:  sra t2,v0,0x10
8002fcdc:  sra t3,v0,0x1f
8002fce0:  mfhi t5
8002fce4:  mflo t4
8002fce8:  move a2,v1
8002fcec:  sra a3,v1,0x1f
8002fcf0:  multu t2,a2
8002fcf4:  lhu t0,0x4(t8)
8002fcf8:  mfhi t9
8002fcfc:  mflo t8
8002fd00:  sw t8,0xd8(sp)
8002fd04:  sw t9,0xdc(sp)
8002fd08:  lw t1,0x8(t6)
8002fd0c:  sll t0,t0,0x10
8002fd10:  sra a0,t0,0x10
8002fd14:  sra a1,t0,0x1f
8002fd18:  move v0,t1
8002fd1c:  sra v1,t1,0x1f
8002fd20:  multu a0,v0
8002fd24:  mfhi t1
8002fd28:  mflo t0
8002fd2c:  nop
8002fd30:  nop
8002fd34:  mult s2,s1
8002fd38:  mflo t6
8002fd3c:  nop
8002fd40:  nop
8002fd44:  mult s0,s3
8002fd48:  mflo s0
8002fd4c:  nop
8002fd50:  nop
8002fd54:  mult t2,a3
8002fd58:  mflo s2
8002fd5c:  nop
8002fd60:  nop
8002fd64:  mult a2,t3
8002fd68:  mflo a3
8002fd6c:  nop
8002fd70:  nop
8002fd74:  mult a0,v1
8002fd78:  sw t6,0xe8(sp)
8002fd7c:  addu t5,t5,t6
8002fd80:  lw t6,0xdc(sp)
8002fd84:  nop
8002fd88:  addu t6,t6,s2
8002fd8c:  mflo t2
8002fd90:  sw t6,0xdc(sp)
8002fd94:  nop
8002fd98:  mult v0,a1
8002fd9c:  move s7,t6
8002fda0:  addu s7,s7,a3
8002fda4:  sw s7,0xdc(sp)
8002fda8:  lw t8,0xd8(sp)
8002fdac:  lw t9,0xdc(sp)
8002fdb0:  addu t5,t5,s0
8002fdb4:  addiu a2,sp,0xa8
8002fdb8:  addiu a0,s6,0x10
8002fdbc:  addu t1,t1,t2
8002fdc0:  lw a1,0xc8(sp)
8002fdc4:  mflo v0
8002fdc8:  addu t1,t1,v0
8002fdcc:  addu t4,t4,t8
8002fdd0:  sltu v0,t4,t8
8002fdd4:  addu t5,t5,t9
8002fdd8:  addu t5,t5,v0
8002fddc:  addu t4,t4,t0
8002fde0:  sltu v0,t4,t0
8002fde4:  addu t5,t5,t1
8002fde8:  addu t5,t5,v0
8002fdec:  srl t4,t4,0xf
8002fdf0:  sll v0,t5,0x11
8002fdf4:  or t4,t4,v0
8002fdf8:  sra t5,t5,0xf
8002fdfc:  jal 0x800434d0
8002fe00:  _move s0,t4
8002fe04:  lw v0,0xa8(sp)
8002fe08:  nop
8002fe0c:  subu v0,zero,v0
8002fe10:  mult v0,s0
8002fe14:  mflo a0
8002fe18:  bgez a0,0x8002fe24
8002fe1c:  _nop
8002fe20:  addiu a0,a0,0xfff
8002fe24:  lw v1,0x48(s4)
8002fe28:  lw v0,0x80(sp)
8002fe2c:  clear a1
8002fe30:  subu v1,v1,v0
8002fe34:  bgez v1,0x8002fe40
8002fe38:  _sra a0,a0,0xc
8002fe3c:  move a1,v1
8002fe40:  lw v0,0xb0(sp)
8002fe44:  nop
8002fe48:  subu v0,zero,v0
8002fe4c:  mult v0,s0
8002fe50:  subu v0,a0,a1
8002fe54:  mflo t4
8002fe58:  bgez t4,0x8002fe64
8002fe5c:  _sw v0,0x50(sp)
8002fe60:  addiu t4,t4,0xfff
8002fe64:  lw v1,0x50(s4)
8002fe68:  lw v0,0x88(sp)
8002fe6c:  clear a0
8002fe70:  subu v1,v1,v0
8002fe74:  bgez v1,0x8002fe80
8002fe78:  _sra t4,t4,0xc
8002fe7c:  move a0,v1
8002fe80:  subu v0,t4,a0
8002fe84:  sw v0,0x58(sp)
8002fe88:  lw a0,0x80(s4)
8002fe8c:  lw a1,0x84(sp)
8002fe90:  nop
8002fe94:  slt v0,a0,a1
8002fe98:  beq v0,zero,0x8002fea4
8002fe9c:  _nop
8002fea0:  move a0,a1
8002fea4:  lw v0,0x84(s4)
8002fea8:  lh v1,0x8c(s4)
8002feac:  subu v0,v0,a0
8002feb0:  mult v0,v1
8002feb4:  lw v1,0xac(sp)
8002feb8:  mflo t0
8002febc:  sll v0,t0,0x7
8002fec0:  nop
8002fec4:  div v0,v1
8002fec8:  mflo a0
8002fecc:  nop
8002fed0:  sw a0,0x54(sp)
8002fed4:  lw v0,0x80(s4)
8002fed8:  nop
8002fedc:  slt v0,v0,a1
8002fee0:  bne v0,zero,0x8002fefc
8002fee4:  _nop
8002fee8:  lw v1,0x4c(s4)
8002feec:  nop
8002fef0:  slt v0,v1,a1
8002fef4:  beq v0,zero,0x8002ff2c
8002fef8:  _subu v0,a1,v1
8002fefc:  lw v0,0x4c(s4)
8002ff00:  lh v1,0x8e(s4)
8002ff04:  subu v0,a1,v0
8002ff08:  mult v0,v1
8002ff0c:  mflo v0
8002ff10:  bgez v0,0x8002ff1c
8002ff14:  _nop
8002ff18:  addiu v0,v0,0x1f
8002ff1c:  sra v0,v0,0x5
8002ff20:  addu v0,a0,v0
8002ff24:  j 0x8002ff48
8002ff28:  _sw v0,0x54(sp)
8002ff2c:  sll v0,v0,0x4
8002ff30:  addu v0,a0,v0
8002ff34:  sw v0,0x54(sp)
8002ff38:  lw v0,0x0(s6)
8002ff3c:  lui v1,0x40
8002ff40:  or v0,v0,v1
8002ff44:  sw v0,0x0(s6)
8002ff48:  lw v0,0x84(sp)
8002ff4c:  nop
8002ff50:  sw v0,0x4c(s4)
8002ff54:  lw t6,0xd0(sp)
8002ff58:  nop
8002ff5c:  beq t6,zero,0x8002ff90
8002ff60:  _nop
8002ff64:  lh a0,0x10(t6)
8002ff68:  nop
8002ff6c:  beq a0,zero,0x8002ff90
8002ff70:  _li v0,0x100
8002ff74:  lw v1,0x54(sp)
8002ff78:  subu v0,v0,a0
8002ff7c:  subu v1,zero,v1
8002ff80:  mult v1,v0
8002ff84:  mflo t6
8002ff88:  j 0x8002ffa0
8002ff8c:  _sra t3,t6,0x7
8002ff90:  lw v0,0x54(sp)
8002ff94:  nop
8002ff98:  subu v0,zero,v0
8002ff9c:  sll t3,v0,0x1
8002ffa0:  lw v0,0x0(s4)
8002ffa4:  lui v1,0x2
8002ffa8:  and v0,v0,v1
8002ffac:  beq v0,zero,0x80030068
8002ffb0:  _nop
8002ffb4:  lw v0,0x90(sp)
8002ffb8:  lw t6,0xc0(sp)
8002ffbc:  nop
8002ffc0:  mult v0,t6
8002ffc4:  lw v0,0x98(sp)
8002ffc8:  mfhi t1
8002ffcc:  mflo t0
8002ffd0:  nop
8002ffd4:  nop
8002ffd8:  mult v0,s8
8002ffdc:  lw v0,0x30(sp)
8002ffe0:  mfhi a1
8002ffe4:  mflo a0
8002ffe8:  nop
8002ffec:  nop
8002fff0:  mult v0,s8
8002fff4:  lw v0,0x38(sp)
8002fff8:  mfhi a3
8002fffc:  mflo a2
80030000:  move t8,t6
80030004:  nop
80030008:  mult v0,t6
8003000c:  sltu t2,t0,a0
80030010:  subu v0,t0,a0
80030014:  subu v1,t1,a1
80030018:  subu v1,v1,t2
8003001c:  srl v0,v0,0x11
80030020:  sll a0,v1,0xf
80030024:  or v0,v0,a0
80030028:  sra v1,v1,0x11
8003002c:  move a1,v0
80030030:  mfhi t9
80030034:  mflo t8
80030038:  sw t8,0xe0(sp)
8003003c:  sw t9,0xe4(sp)
80030040:  addu v0,a2,t8
80030044:  sltu a0,v0,t8
80030048:  addu v1,a3,t9
8003004c:  addu v1,v1,a0
80030050:  srl v0,v0,0xe
80030054:  sll a0,v1,0x12
80030058:  or v0,v0,a0
8003005c:  sra v1,v1,0xe
80030060:  j 0x80030078
80030064:  _move t0,v0
80030068:  lw v0,0x90(sp)
8003006c:  lw v1,0x38(sp)
80030070:  sra a1,v0,0x5
80030074:  sra t0,v1,0x2
80030078:  lw t9,0xbc(sp)
8003007c:  nop
80030080:  lh a0,0x2(t9)
80030084:  nop
80030088:  bgez a0,0x80030094
8003008c:  _move v0,a0
80030090:  subu v0,zero,v0
80030094:  sll v1,v0,0x6
80030098:  slt v0,v1,t3
8003009c:  beq v0,zero,0x800300a8
800300a0:  _move t4,t3
800300a4:  move t4,v1
800300a8:  bltz a0,0x80030104
800300ac:  _move s0,t4
800300b0:  lw v0,0x0(s4)
800300b4:  lui v1,0x1
800300b8:  and v0,v0,v1
800300bc:  beq v0,zero,0x800300fc
800300c0:  _nop
800300c4:  lw t6,0xbc(sp)
800300c8:  nop
800300cc:  lb v0,0xe(t6)
800300d0:  nop
800300d4:  blez v0,0x800300f4
800300d8:  _subu v0,zero,t0
800300dc:  sra v1,v0,0x2
800300e0:  slt v0,t4,v1
800300e4:  beq v0,zero,0x80030138
800300e8:  _move t4,v1
800300ec:  j 0x80030138
800300f0:  _move s0,t4
800300f4:  j 0x80030138
800300f8:  _subu s0,zero,s0
800300fc:  j 0x80030138
80030100:  _clear s0
80030104:  blez t0,0x80030124
80030108:  _subu v1,zero,t0
8003010c:  subu t4,zero,s0
80030110:  slt v0,v1,t4
80030114:  beq v0,zero,0x80030138
80030118:  _move s0,v1
8003011c:  j 0x80030134
80030120:  _move v1,t4
80030124:  slt v0,s0,v1
80030128:  beq v0,zero,0x80030134
8003012c:  _nop
80030130:  move v1,s0
80030134:  move s0,v1
80030138:  lw s7,0xd0(sp)
8003013c:  nop
80030140:  beq s7,zero,0x80030184
80030144:  _nop
80030148:  lh a0,0x12(s7)
8003014c:  nop
80030150:  beq a0,zero,0x80030184
80030154:  _sra v1,t0,0x8
80030158:  bgez v1,0x80030164
8003015c:  _move v0,v1
80030160:  subu v0,zero,v0
80030164:  mult v1,v0
80030168:  mflo t6
8003016c:  nop
80030170:  nop
80030174:  mult t6,a0
80030178:  mflo s7
8003017c:  sra v0,s7,0xc
80030180:  subu s0,s0,v0
80030184:  blez a1,0x800301a4
80030188:  _subu v1,zero,a1
8003018c:  subu a0,zero,t3
80030190:  slt v0,v1,a0
80030194:  beq v0,zero,0x800301b8
80030198:  _move t0,v1
8003019c:  j 0x800301b4
800301a0:  _move v1,a0
800301a4:  slt v0,t3,v1
800301a8:  beq v0,zero,0x800301b8
800301ac:  _move t0,v1
800301b0:  move v1,t3
800301b4:  move t0,v1
800301b8:  mult s8,s0
800301bc:  lw t6,0xc0(sp)
800301c0:  mflo t1
800301c4:  nop
800301c8:  nop
800301cc:  mult t6,t0
800301d0:  lw a2,0x50(sp)
800301d4:  mflo v0
800301d8:  nop
800301dc:  nop
800301e0:  mult t6,s0
800301e4:  lw a1,0x60(sp)
800301e8:  lw a0,0x64(sp)
800301ec:  mflo t2
800301f0:  lw v1,0x68(sp)
800301f4:  nop
800301f8:  mult s8,t0
800301fc:  lw a3,0x58(sp)
80030200:  sra a1,a1,0x3
80030204:  sra a0,a0,0x3
80030208:  sra v1,v1,0x3
8003020c:  addu v0,t1,v0
80030210:  sra v0,v0,0xc
80030214:  addu a2,a2,v0
80030218:  sw a2,0x50(sp)
8003021c:  mflo t0
80030220:  subu v0,t2,t0
80030224:  sra v0,v0,0xc
80030228:  addu a3,a3,v0
8003022c:  sw a3,0x58(sp)
80030230:  gte_ldR11R12 a1
80030234:  gte_ldR22R23 a0
80030238:  gte_ldR33 v1
8003023c:  li a0,0x7fff
80030240:  sra v1,a2,0x3
80030244:  slt v0,v1,a0
80030248:  beq v0,zero,0x80030254
8003024c:  _li t0,-0x8000
80030250:  move a0,v1
80030254:  slt v0,t0,a0
80030258:  beq v0,zero,0x80030264
8003025c:  _nop
80030260:  move t0,a0
80030264:  lw v0,0x54(sp)
80030268:  li a0,0x7fff
8003026c:  sra v1,v0,0x3
80030270:  slt v0,v1,a0
80030274:  beq v0,zero,0x80030280
80030278:  _li a1,-0x8000
8003027c:  move a0,v1
80030280:  slt v0,a1,a0
80030284:  beq v0,zero,0x80030290
80030288:  _nop
8003028c:  move a1,a0
80030290:  li a0,0x7fff
80030294:  sra a3,a3,0x3
80030298:  slt v0,a3,a0
8003029c:  beq v0,zero,0x800302a8
800302a0:  _li v1,-0x8000
800302a4:  move a0,a3
800302a8:  slt v0,v1,a0
800302ac:  beq v0,zero,0x800302b8
800302b0:  _nop
800302b4:  move v1,a0
800302b8:  ldsv_ t0,a1,v1
800302c4:  nOP12
800302d0:  lw v0,0x20(sp)
800302d4:  addiu a1,sp,0x20
800302d8:  addu v0,v0,a2
800302dc:  sw v0,0x20(sp)
800302e0:  lw v0,0x4(a1)
800302e4:  addiu a2,sp,0x50
800302e8:  lw a0,0x4(a2)
800302ec:  lw v1,0x8(a1)
800302f0:  addu v0,v0,a0
800302f4:  sw v0,0x4(a1)
800302f8:  lw v0,0x8(a2)
800302fc:  nop
80030300:  addu v1,v1,v0
80030304:  sw v1,0x8(a1)
80030308:  gte_stMAC1 v1
8003030c:  lw v0,0x10(sp)
80030310:  nop
80030314:  addu v0,v0,v1
80030318:  sw v0,0x10(sp)
8003031c:  gte_stMAC2 v1
80030320:  lw v0,0x14(sp)
80030324:  nop
80030328:  addu v0,v0,v1
8003032c:  sw v0,0x14(sp)
80030330:  gte_stMAC3 v1
80030334:  lw v0,0x18(sp)
80030338:  lw t6,0xd0(sp)
8003033c:  addu v0,v0,v1
80030340:  beq t6,zero,0x8003037c
80030344:  _sw v0,0x18(sp)
80030348:  lh v1,0x16(t6)
8003034c:  nop
80030350:  beq v1,zero,0x8003037c
80030354:  _li v0,0x7
80030358:  beq v1,v0,0x8003037c
8003035c:  _move a0,s4
80030360:  lw v0,0x730(gp)
80030364:  li a1,0x9
80030368:  jalr v0
8003036c:  _addiu a2,sp,0x70
80030370:  j 0x8003037c
80030374:  _nop
80030378:  sw v0,0x4c(s4)
8003037c:  lw v0,0x90(sp)
80030380:  nop
80030384:  mult s8,v0
80030388:  lw t6,0xc0(sp)
8003038c:  lw v0,0x98(sp)
80030390:  mflo v1
80030394:  nop
80030398:  nop
8003039c:  mult t6,v0
800303a0:  mflo t0
800303a4:  addu v0,v1,t0
800303a8:  bgez v0,0x800303b4
800303ac:  _nop
800303b0:  addiu v0,v0,0xfff
800303b4:  lw v1,0x94(s4)
800303b8:  sra v0,v0,0xc
800303bc:  mult v0,v1
800303c0:  mflo v1
800303c4:  bgez v1,0x800303d8
800303c8:  _sw v0,0x98(s4)
800303cc:  lui v0,0x7
800303d0:  ori v0,v0,0xffff
800303d4:  addu v1,v1,v0
800303d8:  lhu v0,0x40(s4)
800303dc:  addiu s5,s5,0x1
800303e0:  sra v1,v1,0x13
800303e4:  subu v0,v0,v1
800303e8:  sh v0,0x40(s4)
800303ec:  slti v0,s5,0x4
800303f0:  bne v0,zero,0x8002fb3c
800303f4:  _addiu v0,s5,0x4
800303f8:  clear s5
800303fc:  li s0,0x10
80030400:  addu v0,s6,s0
80030404:  lw a0,0xec(v0)
80030408:  addiu s0,s0,0x4
8003040c:  jal 0x8001d708
80030410:  _addiu s5,s5,0x1
80030414:  slti v0,s5,0x4
80030418:  bne v0,zero,0x80030400
8003041c:  _addiu a0,s6,0x10
80030420:  addiu a1,sp,0x20
80030424:  jal 0x80043358
80030428:  _move a2,a1
8003042c:  j 0x8003076c
80030430:  _nop
80030434:  lw v0,0x5c(s6)
80030438:  clear s5
8003043c:  addiu s0,sp,0x90
80030440:  move s2,s1
80030444:  addiu s3,sp,0x60
80030448:  addiu s1,v0,0x4
8003044c:  andi v0,s5,0x1
80030450:  beq v0,zero,0x80030464
80030454:  _nop
80030458:  lw v0,0x0(s1)
8003045c:  j 0x80030470
80030460:  _sw v0,0x90(sp)
80030464:  lw v0,0xc(s1)
80030468:  nop
8003046c:  sw v0,0x90(sp)
80030470:  lw v0,0x0(s0)
80030474:  andi v0,s5,0x2
80030478:  beq v0,zero,0x8003048c
8003047c:  _nop
80030480:  lw v0,0x4(s1)
80030484:  j 0x80030498
80030488:  _sw v0,0x4(s0)
8003048c:  lw v0,0x10(s1)
80030490:  nop
80030494:  sw v0,0x4(s0)
80030498:  lw v0,0x4(s0)
8003049c:  andi v0,s5,0x4
800304a0:  beq v0,zero,0x800304b4
800304a4:  _nop
800304a8:  lw v0,0x8(s1)
800304ac:  j 0x800304c0
800304b0:  _sw v0,0x8(s0)
800304b4:  lw v0,0x14(s1)
800304b8:  nop
800304bc:  sw v0,0x8(s0)
800304c0:  addiu a0,s6,0x10
800304c4:  move a1,s0
800304c8:  lw v0,0x8(s0)
800304cc:  jal 0x80043408
800304d0:  _move a2,s0
800304d4:  move a0,s6
800304d8:  move a1,s0
800304dc:  clear a2
800304e0:  jal 0x8001d748
800304e4:  _addiu a3,sp,0xb8
800304e8:  lw v1,0x94(sp)
800304ec:  nop
800304f0:  subu v1,v1,v0
800304f4:  blez v1,0x800306c8
800304f8:  _nop
800304fc:  lw v0,0x80(s6)
80030500:  nop
80030504:  subu v0,zero,v0
80030508:  bgez v0,0x80030518
8003050c:  _sra a1,v0,0x2
80030510:  addiu v0,v0,0x3
80030514:  sra a1,v0,0x2
80030518:  slti v0,a1,-0xb40
8003051c:  bne v0,zero,0x80030538
80030520:  _li a2,0xb40
80030524:  slt v0,a2,a1
80030528:  bne v0,zero,0x8003053c
8003052c:  _move a0,a2
80030530:  j 0x8003053c
80030534:  _move a0,a1
80030538:  li a0,-0xb40
8003053c:  sw a0,0x60(sp)
80030540:  lw v0,0x88(s6)
80030544:  nop
80030548:  subu v0,zero,v0
8003054c:  bgez v0,0x8003055c
80030550:  _sra a0,v0,0x2
80030554:  addiu v0,v0,0x3
80030558:  sra a0,v0,0x2
8003055c:  slti v0,a0,-0xb40
80030560:  bne v0,zero,0x8003057c
80030564:  _li a1,0xb40
80030568:  slt v0,a1,a0
8003056c:  bne v0,zero,0x80030580
80030570:  _move t0,a1
80030574:  j 0x80030580
80030578:  _move t0,a0
8003057c:  li t0,-0xb40
80030580:  subu v1,zero,v1
80030584:  sw t0,0x68(sp)
80030588:  sw v1,0x64(sp)
8003058c:  lw v0,0x84(s6)
80030590:  nop
80030594:  blez v0,0x800305a4
80030598:  _sra v0,v0,0x2
8003059c:  subu v0,v1,v0
800305a0:  sw v0,0x64(sp)
800305a4:  lw a0,0x90(sp)
800305a8:  lw a1,0x48(s6)
800305ac:  lw v1,0x94(sp)
800305b0:  lw a2,0x4c(s6)
800305b4:  lw v0,0x98(sp)
800305b8:  lw a3,0x50(s6)
800305bc:  subu a0,a0,a1
800305c0:  sra a0,a0,0x3
800305c4:  subu v1,v1,a2
800305c8:  sra v1,v1,0x3
800305cc:  subu v0,v0,a3
800305d0:  sra v0,v0,0x3
800305d4:  gte_ldR11R12 a0
800305d8:  gte_ldR22R23 v1
800305dc:  gte_ldR33 v0
800305e0:  lw a1,0x60(sp)
800305e4:  lw v1,0x64(sp)
800305e8:  sra v0,t0,0x3
800305ec:  sra a0,a1,0x3
800305f0:  sra v1,v1,0x3
800305f4:  ldsv_ a0,v1,v0
80030600:  nOP12
8003060c:  lw v0,0x20(sp)
80030610:  lw a0,0x4(s2)
80030614:  addu v0,v0,a1
80030618:  sw v0,0x20(sp)
8003061c:  lw v0,0x4(s3)
80030620:  lw v1,0x8(s2)
80030624:  addu a0,a0,v0
80030628:  sw a0,0x4(s2)
8003062c:  lw v0,0x8(s3)
80030630:  nop
80030634:  addu v1,v1,v0
80030638:  sw v1,0x8(s2)
8003063c:  gte_stMAC1 v1
80030640:  lw v0,0x10(sp)
80030644:  nop
80030648:  addu v0,v0,v1
8003064c:  sw v0,0x10(sp)
80030650:  gte_stMAC2 v1
80030654:  lw v0,0x14(sp)
80030658:  nop
8003065c:  addu v0,v0,v1
80030660:  sw v0,0x14(sp)
80030664:  gte_stMAC3 v1
80030668:  lw v0,0x18(sp)
8003066c:  lw a0,0xb8(sp)
80030670:  addu v0,v0,v1
80030674:  beq a0,zero,0x800306a4
80030678:  _sw v0,0x18(sp)
8003067c:  lh v1,0x16(a0)
80030680:  nop
80030684:  beq v1,zero,0x800306a4
80030688:  _li v0,0x7
8003068c:  beq v1,v0,0x800306a4
80030690:  _move a0,s6
80030694:  lw v0,0x730(gp)
80030698:  li a1,0x9
8003069c:  jalr v0
800306a0:  _addiu a2,sp,0x90
800306a4:  lw v0,0x84(s6)
800306a8:  nop
800306ac:  slti v0,v0,0x4c01
800306b0:  bne v0,zero,0x800306c8
800306b4:  _lui v1,0x40
800306b8:  lw v0,0x0(s6)
800306bc:  nop
800306c0:  or v0,v0,v1
800306c4:  sw v0,0x0(s6)
800306c8:  addiu s5,s5,0x1
800306cc:  slti v0,s5,0x8
800306d0:  bne v0,zero,0x80030450
800306d4:  _andi v0,s5,0x1
800306d8:  addiu s0,s6,0x10
800306dc:  move a0,s0
800306e0:  addiu a1,sp,0x10
800306e4:  jal 0x8004352c
800306e8:  _move a2,a1
800306ec:  li s5,0x4
800306f0:  lw a0,0xec(s0)
800306f4:  nop
800306f8:  lw v1,0x98(a0)
800306fc:  lw v0,0x84(a0)
80030700:  move a1,v1
80030704:  bgez v1,0x80030710
80030708:  _sw v0,0x4c(a0)
8003070c:  addiu a1,v1,0x3f
80030710:  sra v0,a1,0x6
80030714:  subu v0,v1,v0
80030718:  bgez v0,0x80030724
8003071c:  _sw v0,0x98(a0)
80030720:  addiu v0,v0,0xfff
80030724:  lw v1,0x94(a0)
80030728:  sra v0,v0,0xc
8003072c:  mult v0,v1
80030730:  mflo v1
80030734:  bgez v1,0x80030744
80030738:  _lui v0,0x7
8003073c:  ori v0,v0,0xffff
80030740:  addu v1,v1,v0
80030744:  lhu v0,0x40(a0)
80030748:  addiu s0,s0,0x4
8003074c:  addiu s5,s5,0x1
80030750:  sra v1,v1,0x13
80030754:  subu v0,v0,v1
80030758:  jal 0x8001d708
8003075c:  _sh v0,0x40(a0)
80030760:  slti v0,s5,0x8
80030764:  bne v0,zero,0x800306f0
80030768:  _nop
8003076c:  lw a1,0x24(sp)
80030770:  lw v0,0x30(gp)
80030774:  nop
80030778:  addu a1,a1,v0
8003077c:  sw a1,0x24(sp)
80030780:  lw a2,0x8c(s6)
80030784:  lw v0,0xdc(s6)
80030788:  nop
8003078c:  mult a2,v0
80030790:  lw v0,0x80(s6)
80030794:  mflo a2
80030798:  nop
8003079c:  nop
800307a0:  mult v0,a2
800307a4:  lw a0,0x20(sp)
800307a8:  mfhi t9
800307ac:  mflo t8
800307b0:  sw t8,0xe0(sp)
800307b4:  sw t9,0xe4(sp)
800307b8:  sra v0,t9,0x0
800307bc:  sra v1,t9,0x1f
800307c0:  subu a0,a0,v0
800307c4:  sw a0,0x20(sp)
800307c8:  lw v0,0x84(s6)
800307cc:  nop
800307d0:  mult v0,a2
800307d4:  mfhi t9
800307d8:  mflo t8
800307dc:  sw t8,0xe0(sp)
800307e0:  sw t9,0xe4(sp)
800307e4:  sra v0,t9,0x0
800307e8:  sra v1,t9,0x1f
800307ec:  subu a1,a1,v0
800307f0:  sw a1,0x24(sp)
800307f4:  lw v0,0x88(s6)
800307f8:  nop
800307fc:  mult v0,a2
80030800:  lw a3,0x28(sp)
80030804:  move a0,s6
80030808:  addiu a1,sp,0x20
8003080c:  addiu a2,sp,0x10
80030810:  mfhi t9
80030814:  mflo t8
80030818:  sw t8,0xe0(sp)
8003081c:  sw t9,0xe4(sp)
80030820:  sra v0,t9,0x0
80030824:  sra v1,t9,0x1f
80030828:  subu a3,a3,v0
8003082c:  jal 0x800173fc
80030830:  _sw a3,0x28(sp)
80030834:  lw v1,0x90(s6)
80030838:  nop
8003083c:  bgez v1,0x80030848
80030840:  _move v0,v1
80030844:  addiu v0,v1,0x1f
80030848:  lw a0,0x94(s6)
8003084c:  sra v0,v0,0x5
80030850:  subu v0,v1,v0
80030854:  sw v0,0x90(s6)
80030858:  bgez a0,0x80030864
8003085c:  _move v0,a0
80030860:  addiu v0,a0,0x1f
80030864:  lw v1,0x98(s6)
80030868:  sra v0,v0,0x5
8003086c:  subu v0,a0,v0
80030870:  sw v0,0x94(s6)
80030874:  bgez v1,0x80030880
80030878:  _move v0,v1
8003087c:  addiu v0,v1,0x1f
80030880:  sra v0,v0,0x5
80030884:  subu v0,v1,v0
80030888:  sw v0,0x98(s6)
8003088c:  clear s5
80030890:  move a1,s6
80030894:  lw a0,0x110(a1)
80030898:  nop
8003089c:  beq a0,zero,0x800308b8
800308a0:  _nop
800308a4:  lh v0,0x6(a0)
800308a8:  lhu v1,0x6(a0)
800308ac:  beq v0,zero,0x800308b8
800308b0:  _addiu v0,v1,-0x1
800308b4:  sh v0,0x6(a0)
800308b8:  addiu s5,s5,0x1
800308bc:  slti v0,s5,0x3
800308c0:  bne v0,zero,0x80030894
800308c4:  _addiu a1,a1,0x4
800308c8:  clear s5
800308cc:  move v1,s6
800308d0:  lhu v0,0x11c(v1)
800308d4:  nop
800308d8:  beq v0,zero,0x800308e4
800308dc:  _addiu v0,v0,-0x1
800308e0:  sh v0,0x11c(v1)
800308e4:  addiu s5,s5,0x1
800308e8:  slti v0,s5,0x3
800308ec:  bne v0,zero,0x800308d0
800308f0:  _addiu v1,v1,0x2
800308f4:  lw v1,0x0(s6)
800308f8:  lui v0,0x80
800308fc:  and v0,v1,v0
80030900:  bne v0,zero,0x80030a58
80030904:  _nop
80030908:  lhu v0,0x120(s6)
8003090c:  nop
80030910:  beq v0,zero,0x8003098c
80030914:  _nop
80030918:  lw v0,0x24(s6)
8003091c:  lw a0,0x48(s6)
80030920:  nop
80030924:  subu v0,v0,a0
80030928:  bgez v0,0x80030934
8003092c:  _nop
80030930:  addiu v0,v0,0x1f
80030934:  lw v1,0x28(s6)
80030938:  lw a1,0x4c(s6)
8003093c:  sra v0,v0,0x5
80030940:  addu v0,a0,v0
80030944:  sw v0,0x48(s6)
80030948:  subu v0,v1,a1
8003094c:  bgez v0,0x80030958
80030950:  _nop
80030954:  addiu v0,v0,0x1f
80030958:  lw v1,0x2c(s6)
8003095c:  lw a0,0x50(s6)
80030960:  sra v0,v0,0x5
80030964:  addu v0,a1,v0
80030968:  sw v0,0x4c(s6)
8003096c:  subu v0,v1,a0
80030970:  bgez v0,0x8003097c
80030974:  _nop
80030978:  addiu v0,v0,0x1f
8003097c:  sra v0,v0,0x5
80030980:  addu v0,a0,v0
80030984:  j 0x80030a58
80030988:  _sw v0,0x50(s6)
8003098c:  lh v0,0x6(s6)
80030990:  nop
80030994:  bltz v0,0x800309a8
80030998:  _lui v0,0x4
8003099c:  and v0,v1,v0
800309a0:  beq v0,zero,0x80030a40
800309a4:  _nop
800309a8:  lw t6,0xbc(sp)
800309ac:  lw v1,0x24(s6)
800309b0:  lw a2,0x48(s6)
800309b4:  lbu a0,0x10(t6)
800309b8:  li v0,0x100
800309bc:  subu v1,v1,a2
800309c0:  subu a1,v0,a0
800309c4:  mult v1,a1
800309c8:  mflo v1
800309cc:  bgez v1,0x800309d8
800309d0:  _nop
800309d4:  addiu v1,v1,0xff
800309d8:  lw v0,0x28(s6)
800309dc:  lw a3,0x4c(s6)
800309e0:  nop
800309e4:  subu v0,v0,a3
800309e8:  mult v0,a1
800309ec:  sra v0,v1,0x8
800309f0:  addu v0,a2,v0
800309f4:  mflo a0
800309f8:  bgez a0,0x80030a04
800309fc:  _sw v0,0x48(s6)
80030a00:  addiu a0,a0,0xff
80030a04:  lw v0,0x2c(s6)
80030a08:  lw a2,0x50(s6)
80030a0c:  nop
80030a10:  subu v0,v0,a2
80030a14:  mult v0,a1
80030a18:  sra v0,a0,0x8
80030a1c:  addu v0,a3,v0
80030a20:  mflo v1
80030a24:  bgez v1,0x80030a30
80030a28:  _sw v0,0x4c(s6)
80030a2c:  addiu v1,v1,0xff
80030a30:  sra v0,v1,0x8
80030a34:  addu v0,a2,v0
80030a38:  j 0x80030a58
80030a3c:  _sw v0,0x50(s6)
80030a40:  lw t6,0x24(s6)
80030a44:  lw s7,0x28(s6)
80030a48:  lw t8,0x2c(s6)
80030a4c:  sw t6,0x48(s6)
80030a50:  sw s7,0x4c(s6)
80030a54:  sw t8,0x50(s6)
80030a58:  lw ra,0x114(sp)
80030a5c:  lw s8,0x110(sp)
80030a60:  lw s7,0x10c(sp)
80030a64:  lw s6,0x108(sp)
80030a68:  lw s5,0x104(sp)
80030a6c:  lw s4,0x100(sp)
80030a70:  lw s3,0xfc(sp)
80030a74:  lw s2,0xf8(sp)
80030a78:  lw s1,0xf4(sp)
80030a7c:  lw s0,0xf0(sp)
80030a80:  jr ra
80030a84:  _addiu sp,sp,0x118
