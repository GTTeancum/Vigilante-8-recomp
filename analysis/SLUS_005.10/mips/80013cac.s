# addr: 0x80013cac  name: FUN_80013cac
80013cac:  addiu sp,sp,-0xf0
80013cb0:  li t4,0x3
80013cb4:  li t3,0x60
80013cb8:  li t2,0x77
80013cbc:  li t1,0x140
80013cc0:  li a0,0x2
80013cc4:  li t0,0x9f
80013cc8:  li a3,0xf0
80013ccc:  addiu v1,sp,0x20
80013cd0:  move a2,v1
80013cd4:  addiu a1,sp,0x60
80013cd8:  sw ra,0xec(sp)
80013cdc:  sw s8,0xe8(sp)
80013ce0:  sw s7,0xe4(sp)
80013ce4:  sw s6,0xe0(sp)
80013ce8:  sw s5,0xdc(sp)
80013cec:  sw s4,0xd8(sp)
80013cf0:  sw s3,0xd4(sp)
80013cf4:  sw s2,0xd0(sp)
80013cf8:  sw s1,0xcc(sp)
80013cfc:  sw s0,0xc8(sp)
80013d00:  sw zero,0xb8(sp)
80013d04:  sb t4,0x3(v1)
80013d08:  sb t3,0x7(v1)
80013d0c:  sb zero,0x4(v1)
80013d10:  sb zero,0x5(v1)
80013d14:  bne v1,a2,0x80013d30
80013d18:  _sb zero,0x6(v1)
80013d1c:  sh zero,0x8(v1)
80013d20:  sh t2,0xa(v1)
80013d24:  sh t1,0xc(v1)
80013d28:  j 0x80013d40
80013d2c:  _sh a0,0xe(v1)
80013d30:  sh t0,0x8(v1)
80013d34:  sh zero,0xa(v1)
80013d38:  sh a0,0xc(v1)
80013d3c:  sh a3,0xe(v1)
80013d40:  lw t5,0x0(v1)
80013d44:  lw t6,0x4(v1)
80013d48:  lw t7,0x8(v1)
80013d4c:  lw t8,0xc(v1)
80013d50:  sw t5,0x10(v1)
80013d54:  sw t6,0x14(v1)
80013d58:  sw t7,0x18(v1)
80013d5c:  sw t8,0x1c(v1)
80013d60:  addiu v1,v1,0x20
80013d64:  sltu v0,v1,a1
80013d68:  bne v0,zero,0x80013d04
80013d6c:  _lui s0,0x8007
80013d70:  addiu s0,s0,-0xdf8
80013d74:  move a0,s0
80013d78:  clear a1
80013d7c:  move a2,a1
80013d80:  li a3,0x140
80013d84:  li s2,0xf0
80013d88:  jal 0x8004f0e4
80013d8c:  _sw s2,0x10(sp)
80013d90:  addiu a0,s0,0x5c
80013d94:  clear a1
80013d98:  move a2,s2
80013d9c:  li a3,0x140
80013da0:  jal 0x8004f0e4
80013da4:  _sw s2,0x10(sp)
80013da8:  addiu a0,s0,0xb8
80013dac:  clear a1
80013db0:  move a2,a1
80013db4:  li a3,0x140
80013db8:  li s1,0x77
80013dbc:  jal 0x8004f0e4
80013dc0:  _sw s1,0x10(sp)
80013dc4:  addiu a0,s0,0x114
80013dc8:  clear a1
80013dcc:  move a2,s2
80013dd0:  li a3,0x140
80013dd4:  jal 0x8004f0e4
80013dd8:  _sw s1,0x10(sp)
80013ddc:  addiu a0,s0,0x170
80013de0:  clear a1
80013de4:  li a2,0x79
80013de8:  li a3,0x140
80013dec:  jal 0x8004f0e4
80013df0:  _sw s1,0x10(sp)
80013df4:  addiu a0,s0,0x1cc
80013df8:  clear a1
80013dfc:  li a2,0x169
80013e00:  li a3,0x140
80013e04:  jal 0x8004f0e4
80013e08:  _sw s1,0x10(sp)
80013e0c:  addiu a0,s0,0x228
80013e10:  clear a1
80013e14:  move a2,a1
80013e18:  li a3,0x9f
80013e1c:  jal 0x8004f0e4
80013e20:  _sw s2,0x10(sp)
80013e24:  addiu a0,s0,0x284
80013e28:  clear a1
80013e2c:  move a2,s2
80013e30:  li a3,0x9f
80013e34:  jal 0x8004f0e4
80013e38:  _sw s2,0x10(sp)
80013e3c:  addiu a0,s0,0x2e0
80013e40:  li a1,0xa1
80013e44:  clear a2
80013e48:  li a3,0x9f
80013e4c:  jal 0x8004f0e4
80013e50:  _sw s2,0x10(sp)
80013e54:  addiu a0,s0,0x33c
80013e58:  li a1,0xa1
80013e5c:  move a2,s2
80013e60:  li a3,0x9f
80013e64:  jal 0x8004f0e4
80013e68:  _sw s2,0x10(sp)
80013e6c:  lui s0,0x8007
80013e70:  addiu s0,s0,-0xa60
80013e74:  move a0,s0
80013e78:  clear a1
80013e7c:  move a2,s2
80013e80:  li a3,0x140
80013e84:  jal 0x8004f198
80013e88:  _sw s2,0x10(sp)
80013e8c:  addiu a0,s0,0x14
80013e90:  clear a1
80013e94:  move a2,a1
80013e98:  li a3,0x140
80013e9c:  jal 0x8004f198
80013ea0:  _sw s2,0x10(sp)
80013ea4:  lui a0,0x8007
80013ea8:  addiu v0,a0,-0x1360
80013eac:  addiu v1,v0,0x4
80013eb0:  sw v1,-0x1360(a0)
80013eb4:  addiu v1,v0,0x10
80013eb8:  sw zero,0x4(v0)
80013ebc:  sw v0,0x8(v0)
80013ec0:  sw v1,0xc(v0)
80013ec4:  addiu v0,v0,0xc
80013ec8:  sw zero,0x4(v0)
80013ecc:  jal 0x80043ef0
80013ed0:  _sw v0,0x8(v0)
80013ed4:  lui v1,0xe100
80013ed8:  ori v1,v1,0x220
80013edc:  lui a0,0x8007
80013ee0:  addiu a0,a0,-0x1128
80013ee4:  addiu a1,a0,0x8
80013ee8:  li v0,0x1
80013eec:  sb v0,0x3(a0)
80013ef0:  li v0,0x3
80013ef4:  sw v1,0x4(a0)
80013ef8:  sb v0,0x3(a1)
80013efc:  li v0,0x62
80013f00:  sb v0,0x7(a1)
80013f04:  li v0,0x140
80013f08:  sh zero,0x10(a0)
80013f0c:  sh v0,0x14(a0)
80013f10:  jal 0x80052344
80013f14:  _sh s2,0x16(a0)
80013f18:  jal 0x80011834
80013f1c:  _clear s8
80013f20:  li t5,0x78
80013f24:  jal 0x800128d4
80013f28:  _sw t5,0xbc(sp)
80013f2c:  jal 0x800251fc
80013f30:  _li a0,0x40
80013f34:  lw t6,0xb8(sp)
80013f38:  nop
80013f3c:  bne t6,zero,0x80013f94
80013f40:  _lui a0,0x8006
80013f44:  jal 0x80011adc
80013f48:  _addiu a0,a0,0x55d4
80013f4c:  move s0,v0
80013f50:  lw v0,0x4(s0)
80013f54:  nop
80013f58:  jalr v0
80013f5c:  _nop
80013f60:  bne v0,zero,0x80013f70
80013f64:  _nop
80013f68:  lui v0,0x8006
80013f6c:  addiu v0,v0,0x5344
80013f70:  addiu a0,sp,0x60
80013f74:  jal 0x800524f4
80013f78:  _move a1,v0
80013f7c:  jal 0x80045088
80013f80:  _move a0,s0
80013f84:  lbu v0,0x60(sp)
80013f88:  nop
80013f8c:  beq v0,zero,0x80014fc0
80013f90:  _nop
80013f94:  lbu v0,0x18(gp)
80013f98:  lbu v1,0x19(gp)
80013f9c:  clear s0
80013fa0:  lui t7,0x8007
80013fa4:  addiu t7,t7,-0xa60
80013fa8:  sw zero,0xb8(sp)
80013fac:  sll v0,v0,0x18
80013fb0:  sra v0,v0,0x18
80013fb4:  sll v1,v1,0x18
80013fb8:  sra v1,v1,0x18
80013fbc:  sh v0,0x1c(t7)
80013fc0:  sh v0,0x8(t7)
80013fc4:  sh v1,0x1e(t7)
80013fc8:  jal 0x80029dec
80013fcc:  _sh v1,0xa(t7)
80013fd0:  ori a0,zero,0xe000
80013fd4:  lui v0,0x8006
80013fd8:  addiu a2,v0,0x567c
80013fdc:  lui v0,0x8006
80013fe0:  addiu a1,v0,0x5674
80013fe4:  li v1,0x1
80013fe8:  sltiu v0,s0,0x2
80013fec:  bne v0,zero,0x80014008
80013ff0:  _addu v0,s0,a1
80013ff4:  addu v0,s0,a2
80013ff8:  lb v0,-0x2(v0)
80013ffc:  nop
80014000:  beq v0,zero,0x80014018
80014004:  _addu v0,s0,a1
80014008:  lb v0,0x0(v0)
8001400c:  nop
80014010:  sllv v0,v1,v0
80014014:  or a0,a0,v0
80014018:  addiu s0,s0,0x1
8001401c:  sltiu v0,s0,0x8
80014020:  bne v0,zero,0x80013fec
80014024:  _sltiu v0,s0,0x2
80014028:  jal 0x800227a4
8001402c:  _nop
80014030:  lui a0,0x8006
80014034:  jal 0x80044360
80014038:  _addiu a0,a0,0x55e4
8001403c:  lui a0,0x8006
80014040:  sw v0,0x5f8(gp)
80014044:  jal 0x80015f80
80014048:  _addiu a0,a0,0x55f4
8001404c:  clear a0
80014050:  jal 0x800165cc
80014054:  _move s0,v0
80014058:  jal 0x80017fd4
8001405c:  _li a0,0x1
80014060:  move a0,s0
80014064:  jal 0x80019034
80014068:  _li a1,0x23
8001406c:  move a0,v0
80014070:  sw a0,0x628(gp)
80014074:  jal 0x8001910c
80014078:  _nop
8001407c:  jal 0x8002a598
80014080:  _nop
80014084:  lb v1,0x15(gp)
80014088:  li v0,0x5
8001408c:  bne v1,v0,0x8001409c
80014090:  _lui a0,0xbb40
80014094:  jal 0x8001714c
80014098:  _ori a0,a0,0xe64d
8001409c:  lbu a1,0x15(gp)
800140a0:  lui v1,0x8007
800140a4:  addiu v0,v1,-0x1110
800140a8:  sh zero,0x210(v0)
800140ac:  sh zero,0x108(v0)
800140b0:  sh zero,-0x1110(v1)
800140b4:  addiu v0,a1,-0x3
800140b8:  sltiu v0,v0,0x2
800140bc:  sll v0,v0,0x1
800140c0:  sw v0,0x10(gp)
800140c4:  beq v0,zero,0x800140d0
800140c8:  _li a0,-0x1
800140cc:  li a0,-0x55
800140d0:  sb a0,0x6cf(gp)
800140d4:  li s7,0x1
800140d8:  sll v0,a1,0x18
800140dc:  bne v0,zero,0x80014128
800140e0:  _move s6,s7
800140e4:  lw a0,0x608(gp)
800140e8:  lui v0,0x8006
800140ec:  lb v0,0x5674(v0)
800140f0:  lb v1,0x600(gp)
800140f4:  sll v0,v0,0x3
800140f8:  addu a0,a0,v0
800140fc:  lw v0,0x8(a0)
80014100:  sll v1,v1,0x4
80014104:  addu v0,v0,v1
80014108:  lw a1,0xc(v0)
8001410c:  lhu v1,0x2(v0)
80014110:  lhu a0,0x4(v0)
80014114:  sh v1,0x6f0(gp)
80014118:  sh a0,0x7dc(gp)
8001411c:  lbu a2,0x1(v0)
80014120:  j 0x80014150
80014124:  _addiu a0,sp,0x60
80014128:  lw v0,0x618(gp)
8001412c:  nop
80014130:  beq v0,zero,0x80014144
80014134:  _addiu a0,sp,0x60
80014138:  lui v0,0x8001
8001413c:  j 0x8001414c
80014140:  _addiu a1,v0,0x29c
80014144:  lui v0,0x8006
80014148:  addiu a1,v0,0x5344
8001414c:  clear a2
80014150:  jal 0x80022ba8
80014154:  _nop
80014158:  jal 0x80012980
8001415c:  _nop
80014160:  sw zero,0xc(gp)
80014164:  jal 0x800212c4
80014168:  _clear a0
8001416c:  lb v0,0x15(gp)
80014170:  nop
80014174:  bne v0,zero,0x800141c0
80014178:  _nop
8001417c:  lui a0,0x8006
80014180:  addiu a0,a0,0x5604
80014184:  jal 0x8001a0ac
80014188:  _clear a1
8001418c:  lw a0,0x628(gp)
80014190:  lui a1,0x8001
80014194:  addiu a1,a1,0x2b0
80014198:  li a2,0x10
8001419c:  jal 0x80019960
800141a0:  _li a3,0xc0
800141a4:  jal 0x800126f0
800141a8:  _nop
800141ac:  lw v0,0x62c(gp)
800141b0:  nop
800141b4:  andi v0,v0,0x40
800141b8:  beq v0,zero,0x800141a4
800141bc:  _nop
800141c0:  lw v0,0x7d0(gp)
800141c4:  lw v1,0x7d4(gp)
800141c8:  sw zero,0xb4(sp)
800141cc:  lw v0,0xe0(v0)
800141d0:  beq v1,zero,0x800141e4
800141d4:  _sw v0,0xb0(sp)
800141d8:  lw v1,0xe0(v1)
800141dc:  nop
800141e0:  sw v1,0xb4(sp)
800141e4:  addiu a0,sp,0xa0
800141e8:  clear a1
800141ec:  move a2,a1
800141f0:  move a3,a1
800141f4:  li v0,0x140
800141f8:  sh v0,0xa4(sp)
800141fc:  li v0,0x1e0
80014200:  sh zero,0xa0(sp)
80014204:  sh zero,0xa2(sp)
80014208:  jal 0x8004f704
8001420c:  _sh v0,0xa6(sp)
80014210:  addiu s2,sp,0xa8
80014214:  li v0,0x1
80014218:  sw zero,0xac(sp)
8001421c:  sw v0,-0x5350(gp)
80014220:  sw zero,0x624(gp)
80014224:  lw t8,0xac(sp)
80014228:  clear s5
8001422c:  sw zero,0xc0(sp)
80014230:  addiu t8,t8,0x1
80014234:  andi v0,t8,0x7
80014238:  bne v0,zero,0x80014264
8001423c:  _sw t8,0xac(sp)
80014240:  jal 0x80043bb4
80014244:  _nop
80014248:  beq v0,zero,0x80014264
8001424c:  _nop
80014250:  lw a0,0x628(gp)
80014254:  jal 0x80012a90
80014258:  _move a1,s5
8001425c:  bne v0,zero,0x80014ebc
80014260:  _nop
80014264:  lw v0,0x618(gp)
80014268:  nop
8001426c:  bne v0,zero,0x80014298
80014270:  _li v1,0x2
80014274:  lw v0,0x1c(gp)
80014278:  nop
8001427c:  beq v0,zero,0x80014294
80014280:  _nop
80014284:  lw v1,-0x5350(gp)
80014288:  lw v0,0xc(gp)
8001428c:  j 0x80014298
80014290:  _subu v1,v1,v0
80014294:  li v1,0x1
80014298:  sw v1,0x18(sp)
8001429c:  lw v0,0x18(sp)
800142a0:  nop
800142a4:  beq v0,zero,0x8001436c
800142a8:  _clear s0
800142ac:  lui v0,0x8006
800142b0:  addiu s4,v0,0x5c28
800142b4:  lui s3,0x80
800142b8:  li s1,0x3
800142bc:  jal 0x800120d4
800142c0:  _nop
800142c4:  lw v0,0xc(gp)
800142c8:  lw v1,0x18(sp)
800142cc:  addiu v0,v0,0x1
800142d0:  addiu v1,v1,-0x1
800142d4:  sw v0,0xc(gp)
800142d8:  sh v0,0x6cc(gp)
800142dc:  bne s0,v1,0x800142e8
800142e0:  _clear a0
800142e4:  lw a0,0x18(sp)
800142e8:  jal 0x8002131c
800142ec:  _nop
800142f0:  lw a0,0xc(gp)
800142f4:  jal 0x80021394
800142f8:  _nop
800142fc:  jal 0x80021678
80014300:  _nop
80014304:  lw v0,0x8(s4)
80014308:  nop
8001430c:  and v0,v0,s3
80014310:  beq v0,zero,0x8001431c
80014314:  _nop
80014318:  subu s6,s1,s6
8001431c:  lw v0,0x7d4(gp)
80014320:  nop
80014324:  beq v0,zero,0x80014344
80014328:  _nop
8001432c:  lw v0,0x20(s4)
80014330:  nop
80014334:  and v0,v0,s3
80014338:  beq v0,zero,0x80014344
8001433c:  _nop
80014340:  subu s7,s1,s7
80014344:  lw v0,0x62c(gp)
80014348:  lw a0,0x630(gp)
8001434c:  lw t5,0xc0(sp)
80014350:  lw v1,0x18(sp)
80014354:  addiu s0,s0,0x1
80014358:  or s5,s5,v0
8001435c:  or t5,t5,a0
80014360:  sltu v1,s0,v1
80014364:  bne v1,zero,0x800142bc
80014368:  _sw t5,0xc0(sp)
8001436c:  lhu a0,0xc(gp)
80014370:  jal 0x800212c4
80014374:  _nop
80014378:  lw a1,0x7d0(gp)
8001437c:  lw v0,0x8(gp)
80014380:  lhu a0,0xc(a1)
80014384:  li v1,0x1
80014388:  subu v1,v1,v0
8001438c:  sw v1,0x8(gp)
80014390:  bne a0,zero,0x800143a0
80014394:  _li v0,0x2
80014398:  j 0x800143c0
8001439c:  _clear s6
800143a0:  bne s6,v0,0x800143c0
800143a4:  _lui v1,0x100
800143a8:  lw v0,0x0(a1)
800143ac:  nop
800143b0:  and v0,v0,v1
800143b4:  beq v0,zero,0x800143c0
800143b8:  _nop
800143bc:  li s6,0x1
800143c0:  lw v1,0x7d4(gp)
800143c4:  nop
800143c8:  beq v1,zero,0x80014408
800143cc:  _nop
800143d0:  lhu v0,0xc(v1)
800143d4:  nop
800143d8:  bne v0,zero,0x800143e8
800143dc:  _li v0,0x2
800143e0:  j 0x80014408
800143e4:  _clear s7
800143e8:  bne s7,v0,0x80014408
800143ec:  _nop
800143f0:  lw v0,0x0(v1)
800143f4:  lui v1,0x100
800143f8:  and v0,v0,v1
800143fc:  beq v0,zero,0x80014408
80014400:  _nop
80014404:  li s7,0x1
80014408:  lw v1,0x10(gp)
8001440c:  nop
80014410:  beq v1,zero,0x80014a00
80014414:  _li v0,0x1
80014418:  bne v1,v0,0x80014434
8001441c:  _li a1,0xf0
80014420:  li a0,0x140
80014424:  li a1,0x78
80014428:  li a2,0xa0
8001442c:  j 0x80014440
80014430:  _li a3,0x3c
80014434:  li a0,0xa0
80014438:  li a2,0x50
8001443c:  li a3,0x78
80014440:  jal 0x8001d994
80014444:  _nop
80014448:  jal 0x800119c0
8001444c:  _clear a0
80014450:  lw a0,0x7d4(gp)
80014454:  nop
80014458:  lw v1,0x0(a0)
8001445c:  lui v0,0x100
80014460:  and v0,v1,v0
80014464:  bne v0,zero,0x8001447c
80014468:  _li v0,0x2
8001446c:  li v0,-0x3
80014470:  and v0,v1,v0
80014474:  sw v0,0x0(a0)
80014478:  li v0,0x2
8001447c:  bne s6,v0,0x800144b0
80014480:  _nop
80014484:  lw v1,0x7d0(gp)
80014488:  nop
8001448c:  lw v0,0x0(v1)
80014490:  lw a0,0xf8(v1)
80014494:  ori v0,v0,0x2
80014498:  sw v0,0x0(v1)
8001449c:  lw t6,0xb0(sp)
800144a0:  nop
800144a4:  lh a1,0x8a(t6)
800144a8:  j 0x800144c0
800144ac:  _nop
800144b0:  lw t7,0xb0(sp)
800144b4:  nop
800144b8:  lh a1,0x8a(t7)
800144bc:  move a0,t7
800144c0:  jal 0x8001db24
800144c4:  _nop
800144c8:  lui v1,0x8007
800144cc:  lui v0,0x8007
800144d0:  addiu t7,v0,-0x980
800144d4:  addiu t6,v1,-0x960
800144d8:  lw t8,0x0(t7)
800144dc:  lw t5,0x4(t7)
800144e0:  sw t8,0x0(t6)
800144e4:  sw t5,0x4(t6)
800144e8:  lw t8,0x8(t7)
800144ec:  lw t5,0xc(t7)
800144f0:  sw t8,0x8(t6)
800144f4:  sw t5,0xc(t6)
800144f8:  lw t8,0x10(t7)
800144fc:  lw t5,0x14(t7)
80014500:  sw t8,0x10(t6)
80014504:  sw t5,0x14(t6)
80014508:  lw t8,0x18(t7)
8001450c:  lw t5,0x1c(t7)
80014510:  sw t8,0x18(t6)
80014514:  sw t5,0x1c(t6)
80014518:  jal 0x80021600
8001451c:  _nop
80014520:  move a0,s2
80014524:  jal 0x8004fa6c
80014528:  _li a1,0x1
8001452c:  lw a1,0x628(gp)
80014530:  lw a3,0x18(sp)
80014534:  lui s1,0x8007
80014538:  addiu s0,s1,-0x1110
8001453c:  move a0,s0
80014540:  jal 0x80019d10
80014544:  _move a2,s2
80014548:  bne v0,zero,0x80014558
8001454c:  _nop
80014550:  beq s8,zero,0x80014630
80014554:  _nop
80014558:  beq s8,zero,0x80014568
8001455c:  _move a0,s8
80014560:  jal 0x80018f7c
80014564:  _move a1,s2
80014568:  lw v1,0x8(gp)
8001456c:  lw a0,0x10(gp)
80014570:  addiu v0,sp,0x18
80014574:  lw a1,0xa8(sp)
80014578:  sll v1,v1,0x4
8001457c:  sll a0,a0,0x5
80014580:  addu v0,v0,a0
80014584:  addu v1,v1,v0
80014588:  addiu v1,v1,-0x18
8001458c:  sll v0,v1,0x8
80014590:  srl v0,v0,0x8
80014594:  sw v0,0xa8(sp)
80014598:  lbu v0,0x3(v1)
8001459c:  nop
800145a0:  sll v0,v0,0x18
800145a4:  or v0,v0,a1
800145a8:  sw v0,0x0(v1)
800145ac:  lw v0,0x8(gp)
800145b0:  li s0,0x1
800145b4:  lui s1,0x8007
800145b8:  addiu s1,s1,-0xddc
800145bc:  lui t8,0x8007
800145c0:  addiu t8,t8,-0xdf8
800145c4:  subu v0,s0,v0
800145c8:  sllv a1,v0,s0
800145cc:  addu a1,a1,v0
800145d0:  sll a1,a1,0x3
800145d4:  subu a1,a1,v0
800145d8:  sll a1,a1,0x2
800145dc:  addu a0,a1,s1
800145e0:  jal 0x800504a8
800145e4:  _addu a1,a1,t8
800145e8:  lw v0,0x8(gp)
800145ec:  lw a0,0xa8(sp)
800145f0:  subu s0,s0,v0
800145f4:  sll v1,s0,0x1
800145f8:  addu v1,v1,s0
800145fc:  sll v1,v1,0x3
80014600:  subu v1,v1,s0
80014604:  sll v1,v1,0x2
80014608:  addu v1,v1,s1
8001460c:  sll v0,v1,0x8
80014610:  srl v0,v0,0x8
80014614:  sw v0,0xa8(sp)
80014618:  lbu v0,0x3(v1)
8001461c:  nop
80014620:  sll v0,v0,0x18
80014624:  or v0,v0,a0
80014628:  j 0x800146fc
8001462c:  _sw v0,0x0(v1)
80014630:  lw v0,0x624(gp)
80014634:  nop
80014638:  beq v0,zero,0x800146fc
8001463c:  _nop
80014640:  lh v0,-0x1110(s1)
80014644:  nop
80014648:  bne v0,zero,0x800146fc
8001464c:  _nop
80014650:  lh v0,0x108(s0)
80014654:  nop
80014658:  bne v0,zero,0x800146fc
8001465c:  _nop
80014660:  lh v0,0x210(s0)
80014664:  nop
80014668:  bne v0,zero,0x800146fc
8001466c:  _li v0,0x4
80014670:  lw v1,0x5ac(gp)
80014674:  nop
80014678:  bne v1,v0,0x800146ec
8001467c:  _nop
80014680:  lw v0,0x24(gp)
80014684:  nop
80014688:  bne v0,zero,0x800146a4
8001468c:  _lui v0,0x8001
80014690:  lb v1,0x15(gp)
80014694:  li v0,0x3
80014698:  bne v1,v0,0x800146ac
8001469c:  _lui v0,0x8001
800146a0:  lui v0,0x8001
800146a4:  j 0x800146b0
800146a8:  _addiu a0,v0,0x2cc
800146ac:  addiu a0,v0,0x2e0
800146b0:  lb v0,0x15(gp)
800146b4:  lui v1,0x8006
800146b8:  addiu a2,v1,0x5674
800146bc:  li v1,0x3
800146c0:  bne v0,v1,0x800146dc
800146c4:  _clear a1
800146c8:  lw v0,0x7d0(gp)
800146cc:  nop
800146d0:  lhu v0,0xc(v0)
800146d4:  nop
800146d8:  sltiu a1,v0,0x1
800146dc:  addu v0,a1,a2
800146e0:  lb a1,0x0(v0)
800146e4:  jal 0x80043df8
800146e8:  _nop
800146ec:  lw a0,0x628(gp)
800146f0:  jal 0x8001392c
800146f4:  _nop
800146f8:  move s8,v0
800146fc:  beq s7,zero,0x80014718
80014700:  _addiu a2,sp,0xa8
80014704:  lw a1,0x10(gp)
80014708:  lw a0,0x7d4(gp)
8001470c:  sll a1,a1,0x1
80014710:  jal 0x8002af98
80014714:  _ori a1,a1,0x1
80014718:  lw a0,0x7d4(gp)
8001471c:  lui a1,0x8007
80014720:  addiu a1,a1,-0x940
80014724:  jal 0x8002b7bc
80014728:  _move a2,s2
8001472c:  lw a1,0x628(gp)
80014730:  lw a3,0x18(sp)
80014734:  lui a0,0x8007
80014738:  addiu a0,a0,-0xf00
8001473c:  jal 0x80019d10
80014740:  _move a2,s2
80014744:  jal 0x8004f580
80014748:  _clear a0
8001474c:  lw v1,0x10(gp)
80014750:  lw v0,0x8(gp)
80014754:  lui t5,0x8007
80014758:  addiu t5,t5,-0xdf8
8001475c:  sll v1,v1,0x2
80014760:  addiu v0,v0,-0x1
80014764:  subu v1,v1,v0
80014768:  sll v0,v1,0x1
8001476c:  addu v0,v0,v1
80014770:  sll v0,v0,0x3
80014774:  subu v0,v0,v1
80014778:  sll v0,v0,0x2
8001477c:  addu v0,v0,t5
80014780:  lh a0,0x0(v0)
80014784:  lh a1,0x2(v0)
80014788:  jal 0x8002a25c
8001478c:  _move a2,s2
80014790:  lw v1,0x8(gp)
80014794:  lw v0,0x10(gp)
80014798:  move a2,s2
8001479c:  lui t6,0x8007
800147a0:  addiu t6,t6,-0xa60
800147a4:  lui t7,0x8007
800147a8:  addiu t7,t7,-0xeb0
800147ac:  lw a3,0x60c(gp)
800147b0:  sll a0,v1,0x2
800147b4:  addu a0,a0,v1
800147b8:  sll a0,a0,0x2
800147bc:  addu a0,a0,t6
800147c0:  sll v0,v0,0x2
800147c4:  addu v0,v0,v1
800147c8:  sll a1,v0,0x1
800147cc:  addu a1,a1,v0
800147d0:  sll a1,a1,0x3
800147d4:  subu a1,a1,v0
800147d8:  sll a1,a1,0x2
800147dc:  addu a1,a1,t7
800147e0:  jal 0x80012828
800147e4:  _addiu a3,a3,0x3ffc
800147e8:  jal 0x800119c0
800147ec:  _li a0,0x1
800147f0:  lw a0,0x7d0(gp)
800147f4:  nop
800147f8:  lw v1,0x0(a0)
800147fc:  lui v0,0x100
80014800:  and v0,v1,v0
80014804:  bne v0,zero,0x8001481c
80014808:  _li v0,0x2
8001480c:  li v0,-0x3
80014810:  and v0,v1,v0
80014814:  sw v0,0x0(a0)
80014818:  li v0,0x2
8001481c:  bne s7,v0,0x80014850
80014820:  _nop
80014824:  lw v1,0x7d4(gp)
80014828:  nop
8001482c:  lw v0,0x0(v1)
80014830:  lw a0,0xf8(v1)
80014834:  ori v0,v0,0x2
80014838:  sw v0,0x0(v1)
8001483c:  lw t8,0xb4(sp)
80014840:  nop
80014844:  lh a1,0x8a(t8)
80014848:  j 0x80014860
8001484c:  _nop
80014850:  lw t5,0xb4(sp)
80014854:  nop
80014858:  lh a1,0x8a(t5)
8001485c:  move a0,t5
80014860:  jal 0x8001db24
80014864:  _nop
80014868:  lui v1,0x8007
8001486c:  lui v0,0x8007
80014870:  addiu t5,v0,-0x980
80014874:  addiu t8,v1,-0x940
80014878:  lw t6,0x0(t5)
8001487c:  lw t7,0x4(t5)
80014880:  sw t6,0x0(t8)
80014884:  sw t7,0x4(t8)
80014888:  lw t6,0x8(t5)
8001488c:  lw t7,0xc(t5)
80014890:  sw t6,0x8(t8)
80014894:  sw t7,0xc(t8)
80014898:  lw t6,0x10(t5)
8001489c:  lw t7,0x14(t5)
800148a0:  sw t6,0x10(t8)
800148a4:  sw t7,0x14(t8)
800148a8:  lw t6,0x18(t5)
800148ac:  lw t7,0x1c(t5)
800148b0:  sw t6,0x18(t8)
800148b4:  sw t7,0x1c(t8)
800148b8:  jal 0x80021600
800148bc:  _nop
800148c0:  move a0,s2
800148c4:  jal 0x8004fa6c
800148c8:  _li a1,0x1
800148cc:  beq s6,zero,0x800148e4
800148d0:  _move a2,s2
800148d4:  lw a1,0x10(gp)
800148d8:  lw a0,0x7d0(gp)
800148dc:  jal 0x8002af98
800148e0:  _sll a1,a1,0x1
800148e4:  lw a0,0x7d0(gp)
800148e8:  lui a1,0x8007
800148ec:  addiu a1,a1,-0x960
800148f0:  jal 0x8002b7bc
800148f4:  _move a2,s2
800148f8:  lw a1,0x628(gp)
800148fc:  lw a3,0x18(sp)
80014900:  lui a0,0x8007
80014904:  addiu a0,a0,-0x1008
80014908:  jal 0x80019d10
8001490c:  _move a2,s2
80014910:  jal 0x800128bc
80014914:  _nop
80014918:  jal 0x8004f580
8001491c:  _clear a0
80014920:  lw v1,0x10(gp)
80014924:  lw v0,0x8(gp)
80014928:  lui t6,0x8007
8001492c:  addiu t6,t6,-0xdf8
80014930:  sll v1,v1,0x2
80014934:  addu v1,v1,v0
80014938:  addiu v1,v1,-0x2
8001493c:  sll v0,v1,0x1
80014940:  addu v0,v0,v1
80014944:  sll v0,v0,0x3
80014948:  subu v0,v0,v1
8001494c:  sll v0,v0,0x2
80014950:  addu v0,v0,t6
80014954:  lh a0,0x0(v0)
80014958:  lh a1,0x2(v0)
8001495c:  jal 0x8002a25c
80014960:  _move a2,s2
80014964:  jal 0x8004fb74
80014968:  _move a0,s2
8001496c:  jal 0x8004f580
80014970:  _clear a0
80014974:  lui t7,0x8006
80014978:  lh v0,0x5c28(t7)
8001497c:  nop
80014980:  slti v0,v0,0x2
80014984:  beq v0,zero,0x80014994
80014988:  _addiu v1,t7,0x5c28
8001498c:  lui v0,0x800
80014990:  or s5,s5,v0
80014994:  lh v0,0x18(v1)
80014998:  nop
8001499c:  slti v0,v0,0x2
800149a0:  beq v0,zero,0x800149b8
800149a4:  _lui v0,0x800
800149a8:  lw t8,0xc0(sp)
800149ac:  nop
800149b0:  or t8,t8,v0
800149b4:  sw t8,0xc0(sp)
800149b8:  lw v0,0x10(gp)
800149bc:  lw v1,0x8(gp)
800149c0:  lui t5,0x8007
800149c4:  addiu t5,t5,-0xdf8
800149c8:  sll v0,v0,0x2
800149cc:  addu v0,v0,v1
800149d0:  sll a0,v0,0x1
800149d4:  addu a0,a0,v0
800149d8:  sll a0,a0,0x3
800149dc:  subu a0,a0,v0
800149e0:  sll a0,a0,0x2
800149e4:  jal 0x8004fbe4
800149e8:  _addu a0,a0,t5
800149ec:  lw a0,0x60c(gp)
800149f0:  jal 0x8004fb74
800149f4:  _addiu a0,a0,0x3ffc
800149f8:  j 0x80014d9c
800149fc:  _nop
80014a00:  lb v0,0x15(gp)
80014a04:  nop
80014a08:  slti v0,v0,0x3
80014a0c:  bne v0,zero,0x80014a2c
80014a10:  _nop
80014a14:  lw a0,0x7d0(gp)
80014a18:  nop
80014a1c:  lhu v0,0xc(a0)
80014a20:  nop
80014a24:  beq v0,zero,0x80014a48
80014a28:  _move s1,s7
80014a2c:  lw a0,0x7d4(gp)
80014a30:  lw s0,0x7d0(gp)
80014a34:  beq a0,zero,0x80014a6c
80014a38:  _move s1,s6
80014a3c:  lw v1,0x0(a0)
80014a40:  j 0x80014a54
80014a44:  _lui v0,0x100
80014a48:  lw v1,0x0(a0)
80014a4c:  lui v0,0x100
80014a50:  lw s0,0x7d4(gp)
80014a54:  and v0,v1,v0
80014a58:  bne v0,zero,0x80014a70
80014a5c:  _li v0,0x2
80014a60:  li v0,-0x3
80014a64:  and v0,v1,v0
80014a68:  sw v0,0x0(a0)
80014a6c:  li v0,0x2
80014a70:  bne s1,v0,0x80014a98
80014a74:  _lui v0,0x100
80014a78:  lw a0,0xf8(s0)
80014a7c:  lw v0,0x0(s0)
80014a80:  lw v1,0xe0(s0)
80014a84:  ori v0,v0,0x2
80014a88:  sw v0,0x0(s0)
80014a8c:  lh a1,0x8a(v1)
80014a90:  j 0x80014ac0
80014a94:  _nop
80014a98:  lw v1,0x0(s0)
80014a9c:  nop
80014aa0:  and v0,v1,v0
80014aa4:  bne v0,zero,0x80014ab4
80014aa8:  _li v0,-0x3
80014aac:  and v0,v1,v0
80014ab0:  sw v0,0x0(s0)
80014ab4:  lw a0,0xe0(s0)
80014ab8:  nop
80014abc:  lh a1,0x8a(a0)
80014ac0:  jal 0x8001db24
80014ac4:  _nop
80014ac8:  lw a0,0x8(gp)
80014acc:  jal 0x800119c0
80014ad0:  _nop
80014ad4:  li a0,0x140
80014ad8:  li a1,0xf0
80014adc:  li a2,0xa0
80014ae0:  jal 0x8001d994
80014ae4:  _li a3,0x78
80014ae8:  jal 0x80021600
80014aec:  _nop
80014af0:  li v0,0x2
80014af4:  bne s1,v0,0x80014b1c
80014af8:  _move a0,s2
80014afc:  lw v0,0x0(s0)
80014b00:  lui v1,0x2000
80014b04:  and v0,v0,v1
80014b08:  bne v0,zero,0x80014b1c
80014b0c:  _nop
80014b10:  jal 0x8002b8d0
80014b14:  _move a0,s0
80014b18:  move a0,s2
80014b1c:  jal 0x8004fa6c
80014b20:  _li a1,0x1
80014b24:  lw t0,0x680(gp)
80014b28:  nop
80014b2c:  beq t0,zero,0x80014b70
80014b30:  _lui a1,0x8007
80014b34:  lw v1,-0x111c(a1)
80014b38:  lw a3,0xa8(sp)
80014b3c:  addiu a2,a1,-0x111c
80014b40:  lbu a0,-0x9(a2)
80014b44:  addiu v0,a2,-0xc
80014b48:  sll v0,v0,0x8
80014b4c:  srl v0,v0,0x8
80014b50:  sw v0,0xa8(sp)
80014b54:  lui v0,0xff00
80014b58:  and v1,v1,v0
80014b5c:  or v1,v1,t0
80014b60:  sll a0,a0,0x18
80014b64:  or a0,a0,a3
80014b68:  sw v1,-0x111c(a1)
80014b6c:  sw a0,-0xc(a2)
80014b70:  sw zero,0x680(gp)
80014b74:  beq s1,zero,0x80014b8c
80014b78:  _move a0,s0
80014b7c:  li a1,0x2
80014b80:  subu a1,a1,s1
80014b84:  jal 0x8002af98
80014b88:  _move a2,s2
80014b8c:  move a0,s0
80014b90:  lui a1,0x8007
80014b94:  addiu a1,a1,-0x980
80014b98:  jal 0x8002b7bc
80014b9c:  _move a2,s2
80014ba0:  lw v0,0x618(gp)
80014ba4:  nop
80014ba8:  beq v0,zero,0x80014c14
80014bac:  _nop
80014bb0:  lw v0,0xc(gp)
80014bb4:  nop
80014bb8:  andi v0,v0,0x3f
80014bbc:  slti v0,v0,0x28
80014bc0:  beq v0,zero,0x80014c2c
80014bc4:  _li v1,0x80
80014bc8:  lw v0,0x628(gp)
80014bcc:  nop
80014bd0:  sb v1,0x4(v0)
80014bd4:  lw v0,0x628(gp)
80014bd8:  nop
80014bdc:  sb v1,0x5(v0)
80014be0:  lw v0,0x628(gp)
80014be4:  nop
80014be8:  sb zero,0x6(v0)
80014bec:  lw a0,0x628(gp)
80014bf0:  lui a1,0x8006
80014bf4:  addiu a1,a1,0x560c
80014bf8:  lui a2,0x8006
80014bfc:  addiu a2,a2,0x5618
80014c00:  li a3,0xa
80014c04:  jal 0x80019c64
80014c08:  _sw s2,0x10(sp)
80014c0c:  j 0x80014c2c
80014c10:  _nop
80014c14:  lw a1,0x628(gp)
80014c18:  lw a3,0x18(sp)
80014c1c:  lui a0,0x8007
80014c20:  addiu a0,a0,-0x1110
80014c24:  jal 0x80019d10
80014c28:  _move a2,s2
80014c2c:  beq s8,zero,0x80014c44
80014c30:  _move a0,s8
80014c34:  jal 0x80018f7c
80014c38:  _addiu a1,sp,0xa8
80014c3c:  j 0x80014ce4
80014c40:  _nop
80014c44:  lw v0,0x624(gp)
80014c48:  nop
80014c4c:  beq v0,zero,0x80014ce4
80014c50:  _lui v0,0x8007
80014c54:  lh v0,-0x1110(v0)
80014c58:  nop
80014c5c:  bne v0,zero,0x80014ce4
80014c60:  _li v0,0x4
80014c64:  lw v1,0x5ac(gp)
80014c68:  nop
80014c6c:  bne v1,v0,0x80014ca4
80014c70:  _nop
80014c74:  lw v0,0x24(gp)
80014c78:  nop
80014c7c:  beq v0,zero,0x80014c8c
80014c80:  _lui v0,0x8001
80014c84:  j 0x80014c94
80014c88:  _addiu a0,v0,0x2cc
80014c8c:  lui v0,0x8001
80014c90:  addiu a0,v0,0x2e0
80014c94:  lui v0,0x8006
80014c98:  lb a1,0x5674(v0)
80014c9c:  jal 0x80043df8
80014ca0:  _nop
80014ca4:  lb v0,0x15(gp)
80014ca8:  nop
80014cac:  bne v0,zero,0x80014cd4
80014cb0:  _nop
80014cb4:  jal 0x800220d4
80014cb8:  _nop
80014cbc:  lui v1,0x8006
80014cc0:  lb v1,0x5674(v1)
80014cc4:  nop
80014cc8:  slti v1,v1,0x6
80014ccc:  xor v0,v0,v1
80014cd0:  sw v0,0x620(gp)
80014cd4:  lw a0,0x628(gp)
80014cd8:  jal 0x8001392c
80014cdc:  _nop
80014ce0:  move s8,v0
80014ce4:  jal 0x800128bc
80014ce8:  _nop
80014cec:  jal 0x8004f580
80014cf0:  _clear a0
80014cf4:  lw v0,0x4(gp)
80014cf8:  li v1,0x1
80014cfc:  lui t6,0x8007
80014d00:  addiu t6,t6,-0xdf8
80014d04:  subu v1,v1,v0
80014d08:  sll v0,v1,0x1
80014d0c:  addu v0,v0,v1
80014d10:  sll v0,v0,0x3
80014d14:  subu v0,v0,v1
80014d18:  sll v0,v0,0x2
80014d1c:  addu v0,v0,t6
80014d20:  lh a1,0x2(v0)
80014d24:  clear a0
80014d28:  jal 0x8002a25c
80014d2c:  _move a2,s2
80014d30:  lui t7,0x8006
80014d34:  lh v0,0x5c28(t7)
80014d38:  nop
80014d3c:  slti v0,v0,0x2
80014d40:  beq v0,zero,0x80014d4c
80014d44:  _lui v0,0x800
80014d48:  or s5,s5,v0
80014d4c:  lw v0,0x8(gp)
80014d50:  lw v1,0x4(gp)
80014d54:  move a2,s2
80014d58:  lui t8,0x8007
80014d5c:  addiu t8,t8,-0xa60
80014d60:  lui t5,0x8007
80014d64:  addiu t5,t5,-0xdf8
80014d68:  lw a3,0x60c(gp)
80014d6c:  sll a0,v0,0x2
80014d70:  addu a0,a0,v0
80014d74:  sll a0,a0,0x2
80014d78:  addu a0,a0,t8
80014d7c:  sll a1,v1,0x1
80014d80:  addu a1,a1,v1
80014d84:  sll a1,a1,0x3
80014d88:  subu a1,a1,v1
80014d8c:  sll a1,a1,0x2
80014d90:  addu a1,a1,t5
80014d94:  jal 0x80012828
80014d98:  _addiu a3,a3,0x3ffc
80014d9c:  beq s8,zero,0x80014e34
80014da0:  _nop
80014da4:  lw v0,0x624(gp)
80014da8:  lw v1,0x18(sp)
80014dac:  nop
80014db0:  addu a0,v0,v1
80014db4:  slti v0,a0,0x12d
80014db8:  sw a0,0x624(gp)
80014dbc:  bne v0,zero,0x80014e34
80014dc0:  _nop
80014dc4:  lb v0,0x15(gp)
80014dc8:  nop
80014dcc:  bne v0,zero,0x80014e04
80014dd0:  _lui v0,0x860
80014dd4:  lui v0,0x840
80014dd8:  and v0,s5,v0
80014ddc:  bne v0,zero,0x80014ebc
80014de0:  _nop
80014de4:  lw v0,0x24(gp)
80014de8:  nop
80014dec:  bne v0,zero,0x80014e34
80014df0:  _slti v0,a0,0x4b1
80014df4:  beq v0,zero,0x80014ebc
80014df8:  _nop
80014dfc:  j 0x80014e34
80014e00:  _nop
80014e04:  lw t6,0xc0(sp)
80014e08:  nop
80014e0c:  or v1,s5,t6
80014e10:  and v0,v1,v0
80014e14:  bne v0,zero,0x80014e28
80014e18:  _lui v0,0x20
80014e1c:  slti v0,a0,0x4b1
80014e20:  bne v0,zero,0x80014e34
80014e24:  _lui v0,0x20
80014e28:  and v1,v1,v0
80014e2c:  j 0x80014ebc
80014e30:  _sw v1,0xb8(sp)
80014e34:  lw t7,0xc0(sp)
80014e38:  nop
80014e3c:  or v1,s5,t7
80014e40:  andi v0,v1,0x100
80014e44:  beq v0,zero,0x80014e7c
80014e48:  _andi v0,v1,0x800
80014e4c:  beq v0,zero,0x80014e74
80014e50:  _li t5,0x78
80014e54:  lw v0,0x18(sp)
80014e58:  lw t8,0xbc(sp)
80014e5c:  nop
80014e60:  subu t8,t8,v0
80014e64:  bgez t8,0x80014224
80014e68:  _sw t8,0xbc(sp)
80014e6c:  j 0x80014eb8
80014e70:  _li v0,0x3
80014e74:  j 0x80014224
80014e78:  _sw t5,0xbc(sp)
80014e7c:  lui v0,0x800
80014e80:  and v0,v1,v0
80014e84:  beq v0,zero,0x80014224
80014e88:  _nop
80014e8c:  lw v0,0x618(gp)
80014e90:  nop
80014e94:  bne v0,zero,0x80014eb8
80014e98:  _li v0,0x3
80014e9c:  lw a0,0x628(gp)
80014ea0:  srl a1,s5,0x1b
80014ea4:  xori a1,a1,0x1
80014ea8:  jal 0x80012a90
80014eac:  _andi a1,a1,0x1
80014eb0:  beq v0,zero,0x80014224
80014eb4:  _li v0,0x3
80014eb8:  sb v0,0x14(gp)
80014ebc:  beq s8,zero,0x80014ecc
80014ec0:  _nop
80014ec4:  jal 0x80018f3c
80014ec8:  _move a0,s8
80014ecc:  lw v0,0x618(gp)
80014ed0:  nop
80014ed4:  beq v0,zero,0x80014f2c
80014ed8:  _nop
80014edc:  jal 0x8001265c
80014ee0:  _nop
80014ee4:  jal 0x800120d4
80014ee8:  _nop
80014eec:  lw v0,0x62c(gp)
80014ef0:  nop
80014ef4:  bne v0,zero,0x80014f10
80014ef8:  _li v0,0x3
80014efc:  lw v0,0x630(gp)
80014f00:  nop
80014f04:  bne v0,zero,0x80014f10
80014f08:  _li v0,0x3
80014f0c:  li v0,0x2
80014f10:  sb v0,0x14(gp)
80014f14:  lui a0,0x8006
80014f18:  addiu a0,a0,0x5968
80014f1c:  jal 0x80011c58
80014f20:  _nop
80014f24:  jal 0x800126c8
80014f28:  _nop
80014f2c:  jal 0x800128bc
80014f30:  _nop
80014f34:  jal 0x80044054
80014f38:  _nop
80014f3c:  lw a0,0x5f8(gp)
80014f40:  jal 0x80044394
80014f44:  _nop
80014f48:  jal 0x80022a1c
80014f4c:  _nop
80014f50:  lw a0,0xb0(sp)
80014f54:  jal 0x800204dc
80014f58:  _nop
80014f5c:  lw v0,0x7d4(gp)
80014f60:  nop
80014f64:  beq v0,zero,0x80014f78
80014f68:  _nop
80014f6c:  lw a0,0xb4(sp)
80014f70:  jal 0x800204dc
80014f74:  _nop
80014f78:  jal 0x8002accc
80014f7c:  _nop
80014f80:  jal 0x80041e80
80014f84:  _nop
80014f88:  lw a0,0x628(gp)
80014f8c:  jal 0x8001356c
80014f90:  _nop
80014f94:  lw a0,0x628(gp)
80014f98:  jal 0x800190d8
80014f9c:  _nop
80014fa0:  jal 0x80011914
80014fa4:  _clear a0
80014fa8:  jal 0x80011914
80014fac:  _li a0,0x1
80014fb0:  jal 0x80016678
80014fb4:  _clear a0
80014fb8:  j 0x80013f18
80014fbc:  _nop
80014fc0:  lw ra,0xec(sp)
80014fc4:  lw s8,0xe8(sp)
80014fc8:  lw s7,0xe4(sp)
80014fcc:  lw s6,0xe0(sp)
80014fd0:  lw s5,0xdc(sp)
80014fd4:  lw s4,0xd8(sp)
80014fd8:  lw s3,0xd4(sp)
80014fdc:  lw s2,0xd0(sp)
80014fe0:  lw s1,0xcc(sp)
80014fe4:  lw s0,0xc8(sp)
80014fe8:  jr ra
80014fec:  _addiu sp,sp,0xf0
