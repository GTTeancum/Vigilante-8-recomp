# addr: 0x80040e5c  name: FUN_80040e5c
80040e5c:  addiu sp,sp,-0x38
80040e60:  sw s6,0x28(sp)
80040e64:  move s6,a0
80040e68:  sw s8,0x30(sp)
80040e6c:  sw s7,0x2c(sp)
80040e70:  sw s5,0x24(sp)
80040e74:  sw s4,0x20(sp)
80040e78:  sw s3,0x1c(sp)
80040e7c:  sw s2,0x18(sp)
80040e80:  sw s1,0x14(sp)
80040e84:  sw s0,0x10(sp)
80040e88:  lw t7,0x8(s6)
80040e8c:  lw v0,0x60c(gp)
80040e90:  lhu s8,0xa(t7)
80040e94:  lhu s7,0x2e(t7)
80040e98:  addiu s3,s6,0x20
80040e9c:  addiu s0,v0,0x40
80040ea0:  sh s7,0x8(sp)
80040ea4:  ldv0 s3
80040eac:  nRTPS
80040eb8:  gte_ldVXY0 0x8(s3)
80040ebc:  gte_ldVZ0 0xc(s3)
80040ec0:  nRTPS
80040ecc:  lw v0,0x18(t7)
80040ed0:  lw v1,0x1c(s6)
80040ed4:  lw a0,0x14(t7)
80040ed8:  addu v0,v0,v1
80040edc:  slt v0,v0,a0
80040ee0:  bne v0,zero,0x80040eec
80040ee4:  _nop
80040ee8:  sw zero,0x18(t7)
80040eec:  lhu v0,0x2c(t7)
80040ef0:  nop
80040ef4:  andi v0,v0,0x2
80040ef8:  beq v0,zero,0x8004166c
80040efc:  _clear s1
80040f00:  lw v1,0x18(t7)
80040f04:  lw a1,0xc(t7)
80040f08:  lw a0,0x1c(s6)
80040f0c:  lw s7,0x10(t7)
80040f10:  sll v0,v1,0x1
80040f14:  addu v0,v0,v1
80040f18:  sll v0,v0,0x2
80040f1c:  addu v0,v0,v1
80040f20:  sll v0,v0,0x2
80040f24:  addu s2,a1,v0
80040f28:  blez a0,0x80041c30
80040f2c:  _sw s7,0x0(sp)
80040f30:  lui s5,0xc00
80040f34:  lui s4,0x3c00
80040f38:  addiu t5,s6,0x38
80040f3c:  addiu t6,s2,-0x34
80040f40:  gte_ldVXY0 -0x8(t5)
80040f44:  gte_ldVZ0 -0x4(t5)
80040f48:  nRTPS
80040f54:  nNCLIP
80040f60:  gte_stMAC0 v0
80040f64:  nop
80040f68:  bgez v0,0x80041638
80040f6c:  _nop
80040f70:  gte_stSZ2 v0
80040f74:  lhu v1,0x8(sp)
80040f78:  nop
80040f7c:  slt v0,v0,v1
80040f80:  beq v0,zero,0x80041548
80040f84:  _nop
80040f88:  gte_stSZ1 v0
80040f8c:  nop
80040f90:  bgtz v0,0x80040fb8
80040f94:  _nop
80040f98:  gte_stSZ2 v0
80040f9c:  nop
80040fa0:  bgtz v0,0x80040fb8
80040fa4:  _nop
80040fa8:  gte_stSZ3 v0
80040fac:  nop
80040fb0:  blez v0,0x8004164c
80040fb4:  _nop
80040fb8:  lw v1,0x20(t7)
80040fbc:  lw s7,0x0(sp)
80040fc0:  sll v0,v1,0x1
80040fc4:  addu v0,v0,v1
80040fc8:  sll v0,v0,0x2
80040fcc:  addu v0,v0,v1
80040fd0:  sll v0,v0,0x4
80040fd4:  addu t4,s7,v0
80040fd8:  gte_stSXY0 0x8(t4)
80040fdc:  gte_stSXY1 0x48(t4)
80040fe0:  gte_stSXY2 0x88(t4)
80040fe4:  gte_stSXY0 v0
80040fe8:  nop
80040fec:  sll v0,v0,0x10
80040ff0:  sra t1,v0,0x10
80040ff4:  gte_stSXY1 v0
80040ff8:  nop
80040ffc:  sll v0,v0,0x10
80041000:  sra v1,v0,0x10
80041004:  slt v0,t1,v1
80041008:  beq v0,zero,0x80041014
8004100c:  _move a0,v1
80041010:  move a0,t1
80041014:  slt v0,v1,t1
80041018:  beq v0,zero,0x80041024
8004101c:  _move t3,a0
80041020:  move v1,t1
80041024:  move t1,v1
80041028:  gte_stSXY2 v0
8004102c:  nop
80041030:  sll v0,v0,0x10
80041034:  sra v1,v0,0x10
80041038:  slt v0,t3,v1
8004103c:  beq v0,zero,0x80041048
80041040:  _move a0,v1
80041044:  move a0,t3
80041048:  slt v0,v1,t1
8004104c:  beq v0,zero,0x80041058
80041050:  _move t3,a0
80041054:  move v1,t1
80041058:  move t1,v1
8004105c:  gte_stSZ1 t2
80041060:  gte_stSZ2 v0
80041064:  nop
80041068:  subu t8,v0,t2
8004106c:  gte_stSZ3 v0
80041070:  nop
80041074:  subu t9,v0,t2
80041078:  sll v0,t2,0x2
8004107c:  addu v0,v0,t8
80041080:  addu t2,v0,t9
80041084:  ldv0 t5
8004108c:  lh a3,0x0(s3)
80041090:  lh a1,-0x10(t5)
80041094:  lh a2,-0x16(t5)
80041098:  lh a0,-0xe(t5)
8004109c:  lh t0,-0x14(t5)
800410a0:  lh v1,-0xc(t5)
800410a4:  addu a1,a3,a1
800410a8:  srl v0,a1,0x1f
800410ac:  addu a1,a1,v0
800410b0:  sra a1,a1,0x1
800410b4:  andi a1,a1,0xffff
800410b8:  addu a0,a2,a0
800410bc:  srl v0,a0,0x1f
800410c0:  addu a0,a0,v0
800410c4:  sra a0,a0,0x1
800410c8:  sll a0,a0,0x10
800410cc:  addu a1,a1,a0
800410d0:  addu v1,t0,v1
800410d4:  srl v0,v1,0x1f
800410d8:  addu v1,v1,v0
800410dc:  sra v1,v1,0x1
800410e0:  gte_ldVXY1 a1
800410e4:  gte_ldVZ1 v1
800410e8:  lh v0,-0x8(t5)
800410ec:  lh v1,-0x6(t5)
800410f0:  addu a3,a3,v0
800410f4:  srl v0,a3,0x1f
800410f8:  addu a3,a3,v0
800410fc:  sra a3,a3,0x1
80041100:  andi a3,a3,0xffff
80041104:  addu a2,a2,v1
80041108:  lh v1,-0x4(t5)
8004110c:  srl v0,a2,0x1f
80041110:  addu a2,a2,v0
80041114:  sra a2,a2,0x1
80041118:  sll a2,a2,0x10
8004111c:  addu a3,a3,a2
80041120:  addu t0,t0,v1
80041124:  srl v0,t0,0x1f
80041128:  addu t0,t0,v0
8004112c:  sra t0,t0,0x1
80041130:  gte_ldVXY2 a3
80041134:  gte_ldVZ2 t0
80041138:  nRTPT
80041144:  lh v1,-0x12(t5)
80041148:  nop
8004114c:  sb v1,0x4(t4)
80041150:  sb v1,0x5(t4)
80041154:  sb v1,0x6(t4)
80041158:  lh v1,-0xa(t5)
8004115c:  nop
80041160:  sb v1,0x44(t4)
80041164:  sb v1,0x45(t4)
80041168:  sb v1,0x46(t4)
8004116c:  lh v1,-0x2(t5)
80041170:  nop
80041174:  sb v1,0x84(t4)
80041178:  sb v1,0x85(t4)
8004117c:  sb v1,0x86(t4)
80041180:  lh v1,0x6(t5)
80041184:  nop
80041188:  sb v1,0xc4(t4)
8004118c:  sb v1,0xc5(t4)
80041190:  sb v1,0xc6(t4)
80041194:  gte_stSXY0 v0
80041198:  nop
8004119c:  sll v0,v0,0x10
800411a0:  sra v1,v0,0x10
800411a4:  slt v0,t3,v1
800411a8:  beq v0,zero,0x800411b4
800411ac:  _move a0,v1
800411b0:  move a0,t3
800411b4:  slti v0,a0,0x140
800411b8:  beq v0,zero,0x8004151c
800411bc:  _slt v0,v1,t1
800411c0:  beq v0,zero,0x800411cc
800411c4:  _nop
800411c8:  move v1,t1
800411cc:  bltz v1,0x8004151c
800411d0:  _nop
800411d4:  gte_stSXY0 s7
800411d8:  nop
800411dc:  sw s7,0xc8(t4)
800411e0:  gte_stSXY1 v0
800411e4:  nop
800411e8:  sw v0,0x3c(t4)
800411ec:  sw v0,0x14(t4)
800411f0:  gte_stSXY2 v0
800411f4:  sra v1,t2,0x5
800411f8:  sll v1,v1,0x2
800411fc:  addu v1,v1,s0
80041200:  sw v0,0x70(t4)
80041204:  sw v0,0x20(t4)
80041208:  lw a0,0x0(v1)
8004120c:  sll v0,t4,0x8
80041210:  srl v0,v0,0x8
80041214:  sll a1,t8,0x1
80041218:  addu a1,t2,a1
8004121c:  sw v0,0x0(v1)
80041220:  sra v1,a1,0x5
80041224:  sll v1,v1,0x2
80041228:  addu v1,v1,s0
8004122c:  addiu v0,t4,0x34
80041230:  or a0,a0,s5
80041234:  sw a0,0x0(t4)
80041238:  lw a0,0x0(v1)
8004123c:  sll v0,v0,0x8
80041240:  srl v0,v0,0x8
80041244:  sll a2,t9,0x1
80041248:  addu a1,a1,a2
8004124c:  sra a1,a1,0x5
80041250:  sw v0,0x0(v1)
80041254:  addu v1,t2,a2
80041258:  sra v1,v1,0x5
8004125c:  sll v1,v1,0x2
80041260:  addu v1,v1,s0
80041264:  or a0,a0,s5
80041268:  sw a0,0x34(t4)
8004126c:  lw a0,0x0(v1)
80041270:  sll a1,a1,0x2
80041274:  addu a1,a1,s0
80041278:  addiu v0,t4,0x68
8004127c:  sll v0,v0,0x8
80041280:  srl v0,v0,0x8
80041284:  sw v0,0x0(v1)
80041288:  or a0,a0,s5
8004128c:  sw a0,0x68(t4)
80041290:  lw v1,0x0(a1)
80041294:  addiu v0,t4,0x9c
80041298:  sll v0,v0,0x8
8004129c:  srl v0,v0,0x8
800412a0:  sw v0,0x0(a1)
800412a4:  or v1,v1,s5
800412a8:  sw v1,0x9c(t4)
800412ac:  lh t0,-0x10(t5)
800412b0:  lh a2,0x0(t5)
800412b4:  lh t1,-0xe(t5)
800412b8:  lh a1,0x2(t5)
800412bc:  lh t2,-0xc(t5)
800412c0:  lh a3,0x4(t5)
800412c4:  addu a0,t0,a2
800412c8:  srl v0,a0,0x1f
800412cc:  addu a0,a0,v0
800412d0:  sra a0,a0,0x1
800412d4:  andi a0,a0,0xffff
800412d8:  addu v0,t1,a1
800412dc:  srl v1,v0,0x1f
800412e0:  addu v0,v0,v1
800412e4:  sra v0,v0,0x1
800412e8:  sll v0,v0,0x10
800412ec:  addu a0,a0,v0
800412f0:  addu v0,t2,a3
800412f4:  srl v1,v0,0x1f
800412f8:  addu v0,v0,v1
800412fc:  sra v0,v0,0x1
80041300:  gte_ldVXY0 a0
80041304:  gte_ldVZ0 v0
80041308:  lh v1,-0x8(t5)
8004130c:  lh t3,-0x6(t5)
80041310:  lh a0,-0x4(t5)
80041314:  addu a2,v1,a2
80041318:  srl v0,a2,0x1f
8004131c:  addu a2,a2,v0
80041320:  sra a2,a2,0x1
80041324:  andi a2,a2,0xffff
80041328:  addu a1,t3,a1
8004132c:  srl v0,a1,0x1f
80041330:  addu a1,a1,v0
80041334:  sra a1,a1,0x1
80041338:  sll a1,a1,0x10
8004133c:  addu a2,a2,a1
80041340:  addu a3,a0,a3
80041344:  srl v0,a3,0x1f
80041348:  addu a3,a3,v0
8004134c:  sra a3,a3,0x1
80041350:  gte_ldVXY1 a2
80041354:  gte_ldVZ1 a3
80041358:  addu t0,t0,v1
8004135c:  srl v0,t0,0x1f
80041360:  addu t0,t0,v0
80041364:  sra t0,t0,0x1
80041368:  andi t0,t0,0xffff
8004136c:  addu t1,t1,t3
80041370:  srl v0,t1,0x1f
80041374:  addu t1,t1,v0
80041378:  sra t1,t1,0x1
8004137c:  sll t1,t1,0x10
80041380:  addu t0,t0,t1
80041384:  addu t2,t2,a0
80041388:  srl v0,t2,0x1f
8004138c:  addu t2,t2,v0
80041390:  sra t2,t2,0x1
80041394:  gte_ldVXY2 t0
80041398:  gte_ldVZ2 t2
8004139c:  nRTPT
800413a8:  lh v1,-0x12(t5)
800413ac:  lh v0,-0xa(t5)
800413b0:  nop
800413b4:  addu v1,v1,v0
800413b8:  srl v0,v1,0x1f
800413bc:  addu v1,v1,v0
800413c0:  sra v1,v1,0x1
800413c4:  sll v0,v1,0x8
800413c8:  addu v0,v0,v1
800413cc:  sll v0,v0,0x8
800413d0:  addu v0,v0,v1
800413d4:  or v0,v0,s4
800413d8:  sw v0,0x38(t4)
800413dc:  sw v0,0x10(t4)
800413e0:  lh v1,-0x12(t5)
800413e4:  lh v0,-0x2(t5)
800413e8:  nop
800413ec:  addu v1,v1,v0
800413f0:  srl v0,v1,0x1f
800413f4:  addu v1,v1,v0
800413f8:  sra v1,v1,0x1
800413fc:  sll v0,v1,0x8
80041400:  addu v0,v0,v1
80041404:  sll v0,v0,0x8
80041408:  addu v0,v0,v1
8004140c:  or v0,v0,s4
80041410:  sw v0,0x6c(t4)
80041414:  sw v0,0x1c(t4)
80041418:  lh v1,-0x2(t5)
8004141c:  lh v0,0x6(t5)
80041420:  nop
80041424:  addu v1,v1,v0
80041428:  srl v0,v1,0x1f
8004142c:  addu v1,v1,v0
80041430:  sra v1,v1,0x1
80041434:  sll v0,v1,0x8
80041438:  addu v0,v0,v1
8004143c:  sll v0,v0,0x8
80041440:  addu v0,v0,v1
80041444:  or v0,v0,s4
80041448:  sw v0,0xb8(t4)
8004144c:  sw v0,0x90(t4)
80041450:  lh v1,-0xa(t5)
80041454:  lh v0,0x6(t5)
80041458:  nop
8004145c:  addu v1,v1,v0
80041460:  srl v0,v1,0x1f
80041464:  addu v1,v1,v0
80041468:  sra v1,v1,0x1
8004146c:  sll v0,v1,0x8
80041470:  addu v0,v0,v1
80041474:  sll v0,v0,0x8
80041478:  addu v0,v0,v1
8004147c:  or v0,v0,s4
80041480:  sw v0,0xac(t4)
80041484:  sw v0,0x5c(t4)
80041488:  lh v1,-0xa(t5)
8004148c:  lh v0,-0x2(t5)
80041490:  nop
80041494:  addu v1,v1,v0
80041498:  srl v0,v1,0x1f
8004149c:  addu v1,v1,v0
800414a0:  sra v1,v1,0x1
800414a4:  sll v0,v1,0x8
800414a8:  addu v0,v0,v1
800414ac:  sll v0,v0,0x8
800414b0:  addu v0,v0,v1
800414b4:  or v0,v0,s4
800414b8:  sw v0,0xa0(t4)
800414bc:  sw v0,0x78(t4)
800414c0:  sw v0,0x50(t4)
800414c4:  sw v0,0x28(t4)
800414c8:  gte_stSXY0 v0
800414cc:  nop
800414d0:  sw v0,0xb0(t4)
800414d4:  sw v0,0x60(t4)
800414d8:  gte_stSXY1 v0
800414dc:  nop
800414e0:  sw v0,0xbc(t4)
800414e4:  sw v0,0x94(t4)
800414e8:  gte_stSXY2 v0
800414ec:  nop
800414f0:  sw v0,0xa4(t4)
800414f4:  sw v0,0x7c(t4)
800414f8:  sw v0,0x54(t4)
800414fc:  sw v0,0x2c(t4)
80041500:  lw v0,0x20(t7)
80041504:  lw v1,0x1c(t7)
80041508:  addiu v0,v0,0x1
8004150c:  beq v0,v1,0x80041518
80041510:  _clear a0
80041514:  move a0,v0
80041518:  sw a0,0x20(t7)
8004151c:  gte_ldVXY0 -0x10(t5)
80041520:  gte_ldVZ0 -0xc(t5)
80041524:  gte_ldVXY1 -0x8(t5)
80041528:  gte_ldVZ1 -0x4(t5)
8004152c:  ldv2 t5
80041534:  nRTPT
80041540:  j 0x8004164c
80041544:  _nop
80041548:  gte_stFLAG v0
8004154c:  nop
80041550:  bltz v0,0x8004164c
80041554:  _nop
80041558:  gte_stSXY0 0x3c(t6)
8004155c:  gte_stSXY1 0x48(t6)
80041560:  gte_stSXY2 0x54(t6)
80041564:  ldv0 t5
8004156c:  nRTPS
80041578:  lh v0,-0x12(t5)
8004157c:  nop
80041580:  sb v0,0x38(t6)
80041584:  sb v0,0x39(t6)
80041588:  sb v0,0x3a(t6)
8004158c:  lh v0,-0xa(t5)
80041590:  nop
80041594:  sb v0,0x44(t6)
80041598:  sb v0,0x45(t6)
8004159c:  sb v0,0x46(t6)
800415a0:  lh v0,-0x2(t5)
800415a4:  nop
800415a8:  sb v0,0x50(t6)
800415ac:  sb v0,0x51(t6)
800415b0:  sb v0,0x52(t6)
800415b4:  lh v0,0x6(t5)
800415b8:  nop
800415bc:  sb v0,0x5c(t6)
800415c0:  sb v0,0x5d(t6)
800415c4:  sb v0,0x5e(t6)
800415c8:  gte_stSXY2 0x60(t6)
800415cc:  nAVSZ4
800415d8:  gte_stIR0 v0
800415dc:  nop
800415e0:  srl v0,v0,0x8
800415e4:  sll v0,v0,0x6
800415e8:  addu v0,s8,v0
800415ec:  sh v0,0x42(t6)
800415f0:  gte_stOTZ v1
800415f4:  nop
800415f8:  srl v1,v1,0x1
800415fc:  sll v1,v1,0x2
80041600:  addu v1,v1,s0
80041604:  lw a0,0x0(v1)
80041608:  addiu t6,t6,0x34
8004160c:  move v0,s2
80041610:  sll v0,v0,0x8
80041614:  srl v0,v0,0x8
80041618:  sw v0,0x0(v1)
8004161c:  or a0,a0,s5
80041620:  sw a0,0x0(t6)
80041624:  lw v0,0x18(t7)
80041628:  addiu s2,s2,0x34
8004162c:  addiu v0,v0,0x1
80041630:  j 0x8004164c
80041634:  _sw v0,0x18(t7)
80041638:  ldv0 t5
80041640:  nRTPS
8004164c:  lw v0,0x1c(s6)
80041650:  addiu s1,s1,0x1
80041654:  addiu t5,t5,0x10
80041658:  slt v0,s1,v0
8004165c:  bne v0,zero,0x80040f40
80041660:  _addiu s3,s3,0x10
80041664:  j 0x80041c30
80041668:  _nop
8004166c:  lw v1,0x18(t7)
80041670:  lw a1,0xc(t7)
80041674:  lw a0,0x1c(s6)
80041678:  sll v0,v1,0x2
8004167c:  addu v0,v0,v1
80041680:  sll v0,v0,0x3
80041684:  blez a0,0x80041c30
80041688:  _addu s2,a1,v0
8004168c:  lui s4,0x900
80041690:  addiu t5,s6,0x38
80041694:  addiu t6,s2,-0x28
80041698:  gte_ldVXY0 -0x8(t5)
8004169c:  gte_ldVZ0 -0x4(t5)
800416a0:  nRTPS
800416ac:  nNCLIP
800416b8:  gte_stMAC0 v0
800416bc:  nop
800416c0:  bgez v0,0x80041c04
800416c4:  _nop
800416c8:  gte_stSZ2 v0
800416cc:  lhu v1,0x8(sp)
800416d0:  nop
800416d4:  slt v0,v0,v1
800416d8:  beq v0,zero,0x80041b50
800416dc:  _nop
800416e0:  gte_stSZ1 v0
800416e4:  nop
800416e8:  bgtz v0,0x80041710
800416ec:  _nop
800416f0:  gte_stSZ2 v0
800416f4:  nop
800416f8:  bgtz v0,0x80041710
800416fc:  _nop
80041700:  gte_stSZ3 v0
80041704:  nop
80041708:  blez v0,0x80041c18
8004170c:  _nop
80041710:  lw v1,0x20(t7)
80041714:  lw a0,0x10(t7)
80041718:  sll v0,v1,0x2
8004171c:  addu v0,v0,v1
80041720:  sll v0,v0,0x5
80041724:  addu t4,a0,v0
80041728:  gte_stSXY0 0x8(t4)
8004172c:  gte_stSXY1 0x38(t4)
80041730:  gte_stSXY2 0x68(t4)
80041734:  gte_stSXY0 v0
80041738:  nop
8004173c:  sll v0,v0,0x10
80041740:  sra t1,v0,0x10
80041744:  gte_stSXY1 v0
80041748:  nop
8004174c:  sll v0,v0,0x10
80041750:  sra v1,v0,0x10
80041754:  slt v0,t1,v1
80041758:  beq v0,zero,0x80041764
8004175c:  _move a0,v1
80041760:  move a0,t1
80041764:  slt v0,v1,t1
80041768:  beq v0,zero,0x80041774
8004176c:  _move t3,a0
80041770:  move v1,t1
80041774:  move t1,v1
80041778:  gte_stSXY2 v0
8004177c:  nop
80041780:  sll v0,v0,0x10
80041784:  sra v1,v0,0x10
80041788:  slt v0,t3,v1
8004178c:  beq v0,zero,0x80041798
80041790:  _move a0,v1
80041794:  move a0,t3
80041798:  slt v0,v1,t1
8004179c:  beq v0,zero,0x800417a8
800417a0:  _move t3,a0
800417a4:  move v1,t1
800417a8:  move t1,v1
800417ac:  gte_stSZ1 t2
800417b0:  gte_stSZ2 v0
800417b4:  nop
800417b8:  subu t8,v0,t2
800417bc:  gte_stSZ3 v0
800417c0:  nop
800417c4:  subu t9,v0,t2
800417c8:  sll v0,t2,0x2
800417cc:  addu v0,v0,t8
800417d0:  addu t2,v0,t9
800417d4:  ldv0 t5
800417dc:  lh a3,0x0(s3)
800417e0:  lh a1,-0x10(t5)
800417e4:  lh a2,-0x16(t5)
800417e8:  lh a0,-0xe(t5)
800417ec:  lh t0,-0x14(t5)
800417f0:  lh v1,-0xc(t5)
800417f4:  addu a1,a3,a1
800417f8:  srl v0,a1,0x1f
800417fc:  addu a1,a1,v0
80041800:  sra a1,a1,0x1
80041804:  andi a1,a1,0xffff
80041808:  addu a0,a2,a0
8004180c:  srl v0,a0,0x1f
80041810:  addu a0,a0,v0
80041814:  sra a0,a0,0x1
80041818:  sll a0,a0,0x10
8004181c:  addu a1,a1,a0
80041820:  addu v1,t0,v1
80041824:  srl v0,v1,0x1f
80041828:  addu v1,v1,v0
8004182c:  sra v1,v1,0x1
80041830:  gte_ldVXY1 a1
80041834:  gte_ldVZ1 v1
80041838:  lh v0,-0x8(t5)
8004183c:  lh v1,-0x6(t5)
80041840:  addu a3,a3,v0
80041844:  srl v0,a3,0x1f
80041848:  addu a3,a3,v0
8004184c:  sra a3,a3,0x1
80041850:  andi a3,a3,0xffff
80041854:  addu a2,a2,v1
80041858:  lh v1,-0x4(t5)
8004185c:  srl v0,a2,0x1f
80041860:  addu a2,a2,v0
80041864:  sra a2,a2,0x1
80041868:  sll a2,a2,0x10
8004186c:  addu a3,a3,a2
80041870:  addu t0,t0,v1
80041874:  srl v0,t0,0x1f
80041878:  addu t0,t0,v0
8004187c:  sra t0,t0,0x1
80041880:  gte_ldVXY2 a3
80041884:  gte_ldVZ2 t0
80041888:  nRTPT
80041894:  lh v1,-0x12(t5)
80041898:  nop
8004189c:  sb v1,0x4(t4)
800418a0:  sb v1,0x5(t4)
800418a4:  sb v1,0x6(t4)
800418a8:  gte_stSXY0 v0
800418ac:  nop
800418b0:  sll v0,v0,0x10
800418b4:  sra v1,v0,0x10
800418b8:  slt v0,t3,v1
800418bc:  beq v0,zero,0x800418c8
800418c0:  _move a0,v1
800418c4:  move a0,t3
800418c8:  slti v0,a0,0x140
800418cc:  beq v0,zero,0x80041b24
800418d0:  _slt v0,v1,t1
800418d4:  beq v0,zero,0x800418e0
800418d8:  _nop
800418dc:  move v1,t1
800418e0:  bltz v1,0x80041b24
800418e4:  _nop
800418e8:  gte_stSXY0 s7
800418ec:  nop
800418f0:  sw s7,0x98(t4)
800418f4:  gte_stSXY1 v0
800418f8:  nop
800418fc:  sw v0,0x30(t4)
80041900:  sw v0,0x10(t4)
80041904:  gte_stSXY2 v0
80041908:  sra v1,t2,0x5
8004190c:  sll v1,v1,0x2
80041910:  addu v1,v1,s0
80041914:  sw v0,0x58(t4)
80041918:  sw v0,0x18(t4)
8004191c:  lw a0,0x0(v1)
80041920:  sll v0,t4,0x8
80041924:  srl v0,v0,0x8
80041928:  sll a1,t8,0x1
8004192c:  addu a1,t2,a1
80041930:  sw v0,0x0(v1)
80041934:  sra v1,a1,0x5
80041938:  sll v1,v1,0x2
8004193c:  addu v1,v1,s0
80041940:  addiu v0,t4,0x28
80041944:  or a0,a0,s4
80041948:  sw a0,0x0(t4)
8004194c:  lw a0,0x0(v1)
80041950:  sll v0,v0,0x8
80041954:  srl v0,v0,0x8
80041958:  sll a2,t9,0x1
8004195c:  addu a1,a1,a2
80041960:  sra a1,a1,0x5
80041964:  sw v0,0x0(v1)
80041968:  addu v1,t2,a2
8004196c:  sra v1,v1,0x5
80041970:  sll v1,v1,0x2
80041974:  addu v1,v1,s0
80041978:  or a0,a0,s4
8004197c:  sw a0,0x28(t4)
80041980:  lw a0,0x0(v1)
80041984:  sll a1,a1,0x2
80041988:  addu a1,a1,s0
8004198c:  addiu v0,t4,0x50
80041990:  sll v0,v0,0x8
80041994:  srl v0,v0,0x8
80041998:  sw v0,0x0(v1)
8004199c:  or a0,a0,s4
800419a0:  sw a0,0x50(t4)
800419a4:  lw v1,0x0(a1)
800419a8:  addiu v0,t4,0x78
800419ac:  sll v0,v0,0x8
800419b0:  srl v0,v0,0x8
800419b4:  sw v0,0x0(a1)
800419b8:  or v1,v1,s4
800419bc:  sw v1,0x78(t4)
800419c0:  lh t0,-0x10(t5)
800419c4:  lh a2,0x0(t5)
800419c8:  lh t1,-0xe(t5)
800419cc:  lh a1,0x2(t5)
800419d0:  lh t2,-0xc(t5)
800419d4:  lh a3,0x4(t5)
800419d8:  addu a0,t0,a2
800419dc:  srl v0,a0,0x1f
800419e0:  addu a0,a0,v0
800419e4:  sra a0,a0,0x1
800419e8:  andi a0,a0,0xffff
800419ec:  addu v0,t1,a1
800419f0:  srl v1,v0,0x1f
800419f4:  addu v0,v0,v1
800419f8:  sra v0,v0,0x1
800419fc:  sll v0,v0,0x10
80041a00:  addu a0,a0,v0
80041a04:  addu v0,t2,a3
80041a08:  srl v1,v0,0x1f
80041a0c:  addu v0,v0,v1
80041a10:  sra v0,v0,0x1
80041a14:  gte_ldVXY0 a0
80041a18:  gte_ldVZ0 v0
80041a1c:  lh v1,-0x8(t5)
80041a20:  lh t3,-0x6(t5)
80041a24:  lh a0,-0x4(t5)
80041a28:  addu a2,v1,a2
80041a2c:  srl v0,a2,0x1f
80041a30:  addu a2,a2,v0
80041a34:  sra a2,a2,0x1
80041a38:  andi a2,a2,0xffff
80041a3c:  addu a1,t3,a1
80041a40:  srl v0,a1,0x1f
80041a44:  addu a1,a1,v0
80041a48:  sra a1,a1,0x1
80041a4c:  sll a1,a1,0x10
80041a50:  addu a2,a2,a1
80041a54:  addu a3,a0,a3
80041a58:  srl v0,a3,0x1f
80041a5c:  addu a3,a3,v0
80041a60:  sra a3,a3,0x1
80041a64:  gte_ldVXY1 a2
80041a68:  gte_ldVZ1 a3
80041a6c:  addu t0,t0,v1
80041a70:  srl v0,t0,0x1f
80041a74:  addu t0,t0,v0
80041a78:  sra t0,t0,0x1
80041a7c:  andi t0,t0,0xffff
80041a80:  addu t1,t1,t3
80041a84:  srl v0,t1,0x1f
80041a88:  addu t1,t1,v0
80041a8c:  sra t1,t1,0x1
80041a90:  sll t1,t1,0x10
80041a94:  addu t0,t0,t1
80041a98:  addu t2,t2,a0
80041a9c:  srl v0,t2,0x1f
80041aa0:  addu t2,t2,v0
80041aa4:  sra t2,t2,0x1
80041aa8:  gte_ldVXY2 t0
80041aac:  gte_ldVZ2 t2
80041ab0:  nRTPT
80041abc:  lw v0,0x4(t4)
80041ac0:  nop
80041ac4:  sw v0,0x7c(t4)
80041ac8:  sw v0,0x54(t4)
80041acc:  sw v0,0x2c(t4)
80041ad0:  gte_stSXY0 v0
80041ad4:  nop
80041ad8:  sw v0,0x88(t4)
80041adc:  sw v0,0x48(t4)
80041ae0:  gte_stSXY1 v0
80041ae4:  nop
80041ae8:  sw v0,0x90(t4)
80041aec:  sw v0,0x70(t4)
80041af0:  gte_stSXY2 v0
80041af4:  nop
80041af8:  sw v0,0x80(t4)
80041afc:  sw v0,0x60(t4)
80041b00:  sw v0,0x40(t4)
80041b04:  sw v0,0x20(t4)
80041b08:  lw v0,0x20(t7)
80041b0c:  lw v1,0x1c(t7)
80041b10:  addiu v0,v0,0x1
80041b14:  beq v0,v1,0x80041b20
80041b18:  _clear a0
80041b1c:  move a0,v0
80041b20:  sw a0,0x20(t7)
80041b24:  gte_ldVXY0 -0x10(t5)
80041b28:  gte_ldVZ0 -0xc(t5)
80041b2c:  gte_ldVXY1 -0x8(t5)
80041b30:  gte_ldVZ1 -0x4(t5)
80041b34:  ldv2 t5
80041b3c:  nRTPT
80041b48:  j 0x80041c18
80041b4c:  _nop
80041b50:  gte_stFLAG v0
80041b54:  nop
80041b58:  bltz v0,0x80041c18
80041b5c:  _nop
80041b60:  gte_stSXY0 0x30(t6)
80041b64:  gte_stSXY1 0x38(t6)
80041b68:  gte_stSXY2 0x40(t6)
80041b6c:  ldv0 t5
80041b74:  nRTPS
80041b80:  lh v0,-0x12(t5)
80041b84:  nop
80041b88:  sb v0,0x2c(t6)
80041b8c:  sb v0,0x2d(t6)
80041b90:  sb v0,0x2e(t6)
80041b94:  gte_stSXY2 0x48(t6)
80041b98:  nAVSZ4
80041ba4:  gte_stIR0 v0
80041ba8:  nop
80041bac:  srl v0,v0,0x8
80041bb0:  sll v0,v0,0x6
80041bb4:  addu v0,s8,v0
80041bb8:  sh v0,0x36(t6)
80041bbc:  gte_stOTZ v1
80041bc0:  nop
80041bc4:  srl v1,v1,0x1
80041bc8:  sll v1,v1,0x2
80041bcc:  addu v1,v1,s0
80041bd0:  lw a0,0x0(v1)
80041bd4:  addiu t6,t6,0x28
80041bd8:  move v0,s2
80041bdc:  sll v0,v0,0x8
80041be0:  srl v0,v0,0x8
80041be4:  sw v0,0x0(v1)
80041be8:  or a0,a0,s4
80041bec:  sw a0,0x0(t6)
80041bf0:  lw v0,0x18(t7)
80041bf4:  addiu s2,s2,0x28
80041bf8:  addiu v0,v0,0x1
80041bfc:  j 0x80041c18
80041c00:  _sw v0,0x18(t7)
80041c04:  ldv0 t5
80041c0c:  nRTPS
80041c18:  lw v0,0x1c(s6)
80041c1c:  addiu s1,s1,0x1
80041c20:  addiu t5,t5,0x10
80041c24:  slt v0,s1,v0
80041c28:  bne v0,zero,0x80041698
80041c2c:  _addiu s3,s3,0x10
80041c30:  lw s8,0x30(sp)
80041c34:  lw s7,0x2c(sp)
80041c38:  lw s6,0x28(sp)
80041c3c:  lw s5,0x24(sp)
80041c40:  lw s4,0x20(sp)
80041c44:  lw s3,0x1c(sp)
80041c48:  lw s2,0x18(sp)
80041c4c:  lw s1,0x14(sp)
80041c50:  lw s0,0x10(sp)
80041c54:  jr ra
80041c58:  _addiu sp,sp,0x38
