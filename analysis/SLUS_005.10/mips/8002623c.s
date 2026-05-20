# addr: 0x8002623c  name: FUN_8002623c
8002623c:  addiu sp,sp,-0x70
80026240:  move t7,a1
80026244:  sw s0,0x48(sp)
80026248:  move s0,a2
8002624c:  sw s6,0x60(sp)
80026250:  move s6,a3
80026254:  li v0,0x1
80026258:  lui t5,0x1f80
8002625c:  lw t5,0x4(t5)
80026260:  sllv t6,v0,a0
80026264:  lui t3,0x8009
80026268:  addiu t3,t3,0x1120
8002626c:  srl v1,s0,0x6
80026270:  sll v1,v1,0x2
80026274:  lui t4,0x1f80
80026278:  lw t4,0xc(t4)
8002627c:  srl a1,t7,0x6
80026280:  sll a1,a1,0x7
80026284:  addu v0,v1,a1
80026288:  addu t8,v0,t3
8002628c:  andi t9,s0,0x3f
80026290:  sw ra,0x68(sp)
80026294:  sw s7,0x64(sp)
80026298:  sw s5,0x5c(sp)
8002629c:  sw s4,0x58(sp)
800262a0:  sw s3,0x54(sp)
800262a4:  sw s2,0x50(sp)
800262a8:  sw s1,0x4c(sp)
800262ac:  lw t0,0x80(t8)
800262b0:  addu a2,t7,t6
800262b4:  srl t2,a2,0x6
800262b8:  sll t2,t2,0x7
800262bc:  addu v1,v1,t2
800262c0:  addu v1,v1,t3
800262c4:  lw a3,0x80(v1)
800262c8:  sll v1,t9,0x1
800262cc:  andi s1,t7,0x3f
800262d0:  sll t1,s1,0x7
800262d4:  addu v0,v1,t1
800262d8:  addu t0,t0,v0
800262dc:  lhu t0,0x0(t0)
800262e0:  andi a2,a2,0x3f
800262e4:  sll a2,a2,0x7
800262e8:  addu v1,v1,a2
800262ec:  addu a3,a3,v1
800262f0:  addu v1,s0,t6
800262f4:  lhu a3,0x0(a3)
800262f8:  srl v0,v1,0x6
800262fc:  sll v0,v0,0x2
80026300:  addu a1,v0,a1
80026304:  addu v0,v0,t2
80026308:  addu v0,v0,t3
8002630c:  lw v0,0x80(v0)
80026310:  addu a1,a1,t3
80026314:  lw a1,0x80(a1)
80026318:  andi v1,v1,0x3f
8002631c:  sll v1,v1,0x1
80026320:  addu t1,v1,t1
80026324:  addu v1,v1,a2
80026328:  addu v0,v0,v1
8002632c:  lhu v1,0x0(v0)
80026330:  sll t2,t6,0x8
80026334:  addu a1,a1,t1
80026338:  sll v0,t7,0x8
8002633c:  addu t3,v0,t5
80026340:  lhu a1,0x0(a1)
80026344:  sll v0,s0,0x8
80026348:  addu t5,v0,t4
8002634c:  addu a2,t3,t2
80026350:  addu v0,t5,t2
80026354:  move s3,t0
80026358:  lui t4,0x1f80
8002635c:  lw t4,0x8(t4)
80026360:  move s5,a3
80026364:  andi t0,t0,0x7ff
80026368:  sll t0,t0,0x3
8002636c:  andi a3,a3,0x7ff
80026370:  sll a3,a3,0x3
80026374:  sh t3,0x18(sp)
80026378:  sh t5,0x1c(sp)
8002637c:  sh a2,0x20(sp)
80026380:  sh t5,0x24(sp)
80026384:  sh t3,0x28(sp)
80026388:  sh v0,0x2c(sp)
8002638c:  sh a2,0x30(sp)
80026390:  sh v0,0x34(sp)
80026394:  move s2,a1
80026398:  move s4,v1
8002639c:  addu t0,t4,t0
800263a0:  addu a3,t4,a3
800263a4:  andi a1,a1,0x7ff
800263a8:  sll a1,a1,0x3
800263ac:  addu a1,t4,a1
800263b0:  andi v1,v1,0x7ff
800263b4:  sll v1,v1,0x3
800263b8:  addu v1,t4,v1
800263bc:  sh t0,0x1a(sp)
800263c0:  sh a3,0x22(sp)
800263c4:  sh a1,0x2a(sp)
800263c8:  bne a0,zero,0x80026d14
800263cc:  _sh v1,0x32(sp)
800263d0:  lw v1,0x80(t8)
800263d4:  sll v0,s1,0x6
800263d8:  addu v0,t9,v0
800263dc:  addu v1,v1,v0
800263e0:  lbu v1,0x2000(v1)
800263e4:  lui v0,0x8009
800263e8:  addiu v0,v0,-0xfe0
800263ec:  sll v1,v1,0x5
800263f0:  addu s1,v1,v0
800263f4:  lhu v0,0x2(s1)
800263f8:  nop
800263fc:  beq v0,zero,0x800288d0
80026400:  _lui s7,0x3480
80026404:  ori s7,s7,0x8080
80026408:  gte_ldRGB s7
8002640c:  lhu v0,0x1e(s1)
80026410:  nop
80026414:  beq v0,zero,0x80026898
80026418:  _nop
8002641c:  gte_ldVXY0 0x20(sp)
80026420:  gte_ldVZ0 0x24(sp)
80026424:  gte_ldVXY1 0x30(sp)
80026428:  gte_ldVZ1 0x34(sp)
8002642c:  gte_ldVXY2 0x18(sp)
80026430:  gte_ldVZ2 0x1c(sp)
80026434:  nRTPT
80026440:  lui s0,0x1f80
80026444:  lw s0,0x18(s0)
80026448:  lhu v1,0x4(s1)
8002644c:  addiu v0,s0,0x50
80026450:  lui at,0x1f80
80026454:  sw v0,0x18(at)
80026458:  sh v1,0xc(s0)
8002645c:  lw v0,0xc(s1)
80026460:  nop
80026464:  sw v0,0x18(s0)
80026468:  lhu v0,0x0(s1)
8002646c:  nop
80026470:  sh v0,0x24(s0)
80026474:  gte_stFLAG v0
80026478:  nop
8002647c:  bltz v0,0x8002669c
80026480:  _nop
80026484:  NCLIP
80026488:  gte_stMAC0 v0
8002648c:  nop
80026490:  bgez v0,0x8002669c
80026494:  _nop
80026498:  gte_stSZ0 v1
8002649c:  gte_stSZ1 a0
800264a0:  nop
800264a4:  slt v0,a0,v1
800264a8:  beq v0,zero,0x800264b4
800264ac:  _nop
800264b0:  move a0,v1
800264b4:  gte_stSZ2 v1
800264b8:  nop
800264bc:  slt v0,v1,a0
800264c0:  beq v0,zero,0x800264d0
800264c4:  _move a1,v1
800264c8:  move v1,a0
800264cc:  move a1,v1
800264d0:  gte_stIR0 v0
800264d4:  nop
800264d8:  srl a2,v0,0x8
800264dc:  stsxy3_gt3 s0
800264e8:  srl a0,s5,0xb
800264ec:  sll a0,a0,0x7
800264f0:  srl v1,s4,0xb
800264f4:  sll v1,v1,0x7
800264f8:  srl v0,s3,0xb
800264fc:  sll v0,v0,0x7
80026500:  gte_ldVXY0 a0
80026504:  gte_ldVXY1 v1
80026508:  gte_ldVXY2 v0
8002650c:  nNCCT
80026518:  lhu v0,0x800(gp)
8002651c:  lui a0,0x1f80
80026520:  lw a0,0x0(a0)
80026524:  sll a2,a2,0x6
80026528:  srl v1,a1,0x3
8002652c:  sll v1,v1,0x2
80026530:  addu v0,v0,a2
80026534:  addu v1,v1,a0
80026538:  sh v0,0xe(s0)
8002653c:  lw a0,0x0(v1)
80026540:  lui a3,0x900
80026544:  sll v0,s0,0x8
80026548:  srl v0,v0,0x8
8002654c:  sw v0,0x0(v1)
80026550:  or a0,a0,a3
80026554:  sw a0,0x0(s0)
80026558:  strgb3_gt3 s0
80026564:  gte_ldVXY0 0x28(sp)
80026568:  gte_ldVZ0 0x2c(sp)
8002656c:  nRTPS
80026578:  lhu v0,0xc(s1)
8002657c:  addiu s0,s0,0x28
80026580:  sh v0,0xc(s0)
80026584:  lw v0,0x0(s1)
80026588:  nop
8002658c:  sw v0,0x18(s0)
80026590:  lhu v0,0x8(s1)
80026594:  nop
80026598:  sh v0,0x24(s0)
8002659c:  gte_stFLAG v0
800265a0:  nop
800265a4:  bltz v0,0x80026668
800265a8:  _nop
800265ac:  NCLIP
800265b0:  gte_stMAC0 v0
800265b4:  nop
800265b8:  blez v0,0x80026668
800265bc:  _nop
800265c0:  gte_stSZ0 v1
800265c4:  gte_stSZ1 a0
800265c8:  nop
800265cc:  slt v0,a0,v1
800265d0:  beq v0,zero,0x800265dc
800265d4:  _nop
800265d8:  move a0,v1
800265dc:  gte_stSZ2 v1
800265e0:  nop
800265e4:  slt v0,v1,a0
800265e8:  beq v0,zero,0x800265f8
800265ec:  _move a1,v1
800265f0:  move v1,a0
800265f4:  move a1,v1
800265f8:  stsxy3_gt3 s0
80026604:  srl v0,s2,0xb
80026608:  sll v0,v0,0x7
8002660c:  gte_ldVXY0 v0
80026610:  nNCCS
8002661c:  lhu v0,0x800(gp)
80026620:  lui a0,0x1f80
80026624:  lw a0,0x0(a0)
80026628:  srl v1,a1,0x3
8002662c:  sll v1,v1,0x2
80026630:  addu v0,v0,a2
80026634:  addu v1,v1,a0
80026638:  sh v0,0xe(s0)
8002663c:  lw a0,0x0(v1)
80026640:  sll v0,s0,0x8
80026644:  srl v0,v0,0x8
80026648:  sw v0,0x0(v1)
8002664c:  or a0,a0,a3
80026650:  sw a0,0x0(s0)
80026654:  strgb3_gt3 s0
80026660:  j 0x800288d0
80026664:  _nop
80026668:  gte_stFLAG v0
8002666c:  nop
80026670:  lui v1,0x2
80026674:  and v0,v0,v1
80026678:  beq v0,zero,0x800288d0
8002667c:  _srl v0,s2,0xb
80026680:  sll v0,v0,0x7
80026684:  gte_ldVXY0 v0
80026688:  nNCCS
80026694:  j 0x8002687c
80026698:  _sw zero,0x10(sp)
8002669c:  gte_stFLAG v0
800266a0:  nop
800266a4:  lui v1,0x2
800266a8:  and v0,v0,v1
800266ac:  beq v0,zero,0x80026728
800266b0:  _srl a0,s5,0xb
800266b4:  sll a0,a0,0x7
800266b8:  srl v1,s4,0xb
800266bc:  sll v1,v1,0x7
800266c0:  srl v0,s3,0xb
800266c4:  sll v0,v0,0x7
800266c8:  gte_ldVXY0 a0
800266cc:  gte_ldVXY1 v1
800266d0:  gte_ldVXY2 v0
800266d4:  nNCCT
800266e0:  li v0,-0x1
800266e4:  sw v0,0x10(sp)
800266e8:  move a0,s0
800266ec:  addiu a1,sp,0x20
800266f0:  addiu a2,sp,0x30
800266f4:  jal 0x80025bc0
800266f8:  _addiu a3,sp,0x18
800266fc:  gte_ldVXY0 0x30(sp)
80026700:  gte_ldVZ0 0x34(sp)
80026704:  gte_ldVXY1 0x18(sp)
80026708:  gte_ldVZ1 0x1c(sp)
8002670c:  gte_ldVXY2 0x28(sp)
80026710:  gte_ldVZ2 0x2c(sp)
80026714:  nRTPT
80026720:  j 0x8002673c
80026724:  _nop
80026728:  gte_ldVXY0 0x28(sp)
8002672c:  gte_ldVZ0 0x2c(sp)
80026730:  nRTPS
8002673c:  lhu v0,0xc(s1)
80026740:  addiu s0,s0,0x28
80026744:  sh v0,0xc(s0)
80026748:  lw v0,0x0(s1)
8002674c:  nop
80026750:  sw v0,0x18(s0)
80026754:  lhu v0,0x8(s1)
80026758:  srl a0,s4,0xb
8002675c:  sll a0,a0,0x7
80026760:  srl v1,s3,0xb
80026764:  sll v1,v1,0x7
80026768:  sh v0,0x24(s0)
8002676c:  srl v0,s2,0xb
80026770:  sll v0,v0,0x7
80026774:  gte_ldVXY0 a0
80026778:  gte_ldVXY1 v1
8002677c:  gte_ldVXY2 v0
80026780:  gte_stFLAG v0
80026784:  nop
80026788:  bltz v0,0x80026854
8002678c:  _nop
80026790:  NCLIP
80026794:  gte_stMAC0 v0
80026798:  nop
8002679c:  blez v0,0x80026854
800267a0:  _nop
800267a4:  gte_stSZ0 v1
800267a8:  gte_stSZ1 a0
800267ac:  nop
800267b0:  slt v0,a0,v1
800267b4:  beq v0,zero,0x800267c0
800267b8:  _nop
800267bc:  move a0,v1
800267c0:  gte_stSZ2 v1
800267c4:  nop
800267c8:  slt v0,v1,a0
800267cc:  beq v0,zero,0x800267dc
800267d0:  _move a1,v1
800267d4:  move v1,a0
800267d8:  move a1,v1
800267dc:  gte_stIR0 v0
800267e0:  nop
800267e4:  srl a2,v0,0x8
800267e8:  stsxy3_gt3 s0
800267f4:  nNCCT
80026800:  lhu v0,0x800(gp)
80026804:  lui a0,0x1f80
80026808:  lw a0,0x0(a0)
8002680c:  sll v1,a2,0x6
80026810:  addu v0,v0,v1
80026814:  srl v1,a1,0x3
80026818:  sll v1,v1,0x2
8002681c:  addu v1,v1,a0
80026820:  sh v0,0xe(s0)
80026824:  lw a0,0x0(v1)
80026828:  sll v0,s0,0x8
8002682c:  srl v0,v0,0x8
80026830:  sw v0,0x0(v1)
80026834:  lui v0,0x900
80026838:  or a0,a0,v0
8002683c:  sw a0,0x0(s0)
80026840:  strgb3_gt3 s0
8002684c:  j 0x800288d0
80026850:  _nop
80026854:  gte_stFLAG v0
80026858:  nop
8002685c:  lui v1,0x2
80026860:  and v0,v0,v1
80026864:  beq v0,zero,0x800288d0
80026868:  _nop
8002686c:  nNCCT
80026878:  sw zero,0x10(sp)
8002687c:  move a0,s0
80026880:  addiu a1,sp,0x30
80026884:  addiu a2,sp,0x18
80026888:  jal 0x80025bc0
8002688c:  _addiu a3,sp,0x28
80026890:  j 0x800288d0
80026894:  _nop
80026898:  gte_ldVXY0 0x18(sp)
8002689c:  gte_ldVZ0 0x1c(sp)
800268a0:  gte_ldVXY1 0x20(sp)
800268a4:  gte_ldVZ1 0x24(sp)
800268a8:  gte_ldVXY2 0x28(sp)
800268ac:  gte_ldVZ2 0x2c(sp)
800268b0:  nRTPT
800268bc:  lui s0,0x1f80
800268c0:  lw s0,0x18(s0)
800268c4:  lhu v1,0x0(s1)
800268c8:  addiu v0,s0,0x50
800268cc:  lui at,0x1f80
800268d0:  sw v0,0x18(at)
800268d4:  sh v1,0xc(s0)
800268d8:  lw v0,0x4(s1)
800268dc:  nop
800268e0:  sw v0,0x18(s0)
800268e4:  lhu v0,0x8(s1)
800268e8:  nop
800268ec:  sh v0,0x24(s0)
800268f0:  gte_stFLAG v0
800268f4:  nop
800268f8:  bltz v0,0x80026b18
800268fc:  _nop
80026900:  NCLIP
80026904:  gte_stMAC0 v0
80026908:  nop
8002690c:  bgez v0,0x80026b18
80026910:  _nop
80026914:  gte_stSZ0 v1
80026918:  gte_stSZ1 a0
8002691c:  nop
80026920:  slt v0,a0,v1
80026924:  beq v0,zero,0x80026930
80026928:  _nop
8002692c:  move a0,v1
80026930:  gte_stSZ2 v1
80026934:  nop
80026938:  slt v0,v1,a0
8002693c:  beq v0,zero,0x8002694c
80026940:  _move a1,v1
80026944:  move v1,a0
80026948:  move a1,v1
8002694c:  gte_stIR0 v0
80026950:  nop
80026954:  srl a2,v0,0x8
80026958:  stsxy3_gt3 s0
80026964:  srl a0,s3,0xb
80026968:  sll a0,a0,0x7
8002696c:  srl v1,s5,0xb
80026970:  sll v1,v1,0x7
80026974:  srl v0,s2,0xb
80026978:  sll v0,v0,0x7
8002697c:  gte_ldVXY0 a0
80026980:  gte_ldVXY1 v1
80026984:  gte_ldVXY2 v0
80026988:  nNCCT
80026994:  lhu v0,0x800(gp)
80026998:  lui a0,0x1f80
8002699c:  lw a0,0x0(a0)
800269a0:  sll a2,a2,0x6
800269a4:  srl v1,a1,0x3
800269a8:  sll v1,v1,0x2
800269ac:  addu v0,v0,a2
800269b0:  addu v1,v1,a0
800269b4:  sh v0,0xe(s0)
800269b8:  lw a0,0x0(v1)
800269bc:  lui a3,0x900
800269c0:  sll v0,s0,0x8
800269c4:  srl v0,v0,0x8
800269c8:  sw v0,0x0(v1)
800269cc:  or a0,a0,a3
800269d0:  sw a0,0x0(s0)
800269d4:  strgb3_gt3 s0
800269e0:  gte_ldVXY0 0x30(sp)
800269e4:  gte_ldVZ0 0x34(sp)
800269e8:  nRTPS
800269f4:  lhu v0,0x4(s1)
800269f8:  addiu s0,s0,0x28
800269fc:  sh v0,0xc(s0)
80026a00:  lw v0,0x8(s1)
80026a04:  nop
80026a08:  sw v0,0x18(s0)
80026a0c:  lhu v0,0xc(s1)
80026a10:  nop
80026a14:  sh v0,0x24(s0)
80026a18:  gte_stFLAG v0
80026a1c:  nop
80026a20:  bltz v0,0x80026ae4
80026a24:  _nop
80026a28:  NCLIP
80026a2c:  gte_stMAC0 v0
80026a30:  nop
80026a34:  blez v0,0x80026ae4
80026a38:  _nop
80026a3c:  gte_stSZ0 v1
80026a40:  gte_stSZ1 a0
80026a44:  nop
80026a48:  slt v0,a0,v1
80026a4c:  beq v0,zero,0x80026a58
80026a50:  _nop
80026a54:  move a0,v1
80026a58:  gte_stSZ2 v1
80026a5c:  nop
80026a60:  slt v0,v1,a0
80026a64:  beq v0,zero,0x80026a74
80026a68:  _move a1,v1
80026a6c:  move v1,a0
80026a70:  move a1,v1
80026a74:  stsxy3_gt3 s0
80026a80:  srl v0,s4,0xb
80026a84:  sll v0,v0,0x7
80026a88:  gte_ldVXY0 v0
80026a8c:  nNCCS
80026a98:  lhu v0,0x800(gp)
80026a9c:  lui a0,0x1f80
80026aa0:  lw a0,0x0(a0)
80026aa4:  srl v1,a1,0x3
80026aa8:  sll v1,v1,0x2
80026aac:  addu v0,v0,a2
80026ab0:  addu v1,v1,a0
80026ab4:  sh v0,0xe(s0)
80026ab8:  lw a0,0x0(v1)
80026abc:  sll v0,s0,0x8
80026ac0:  srl v0,v0,0x8
80026ac4:  sw v0,0x0(v1)
80026ac8:  or a0,a0,a3
80026acc:  sw a0,0x0(s0)
80026ad0:  strgb3_gt3 s0
80026adc:  j 0x800288d0
80026ae0:  _nop
80026ae4:  gte_stFLAG v0
80026ae8:  nop
80026aec:  lui v1,0x2
80026af0:  and v0,v0,v1
80026af4:  beq v0,zero,0x800288d0
80026af8:  _srl v0,s4,0xb
80026afc:  sll v0,v0,0x7
80026b00:  gte_ldVXY0 v0
80026b04:  nNCCS
80026b10:  j 0x80026cf8
80026b14:  _sw zero,0x10(sp)
80026b18:  gte_stFLAG v0
80026b1c:  nop
80026b20:  lui v1,0x2
80026b24:  and v0,v0,v1
80026b28:  beq v0,zero,0x80026ba4
80026b2c:  _srl a0,s3,0xb
80026b30:  sll a0,a0,0x7
80026b34:  srl v1,s5,0xb
80026b38:  sll v1,v1,0x7
80026b3c:  srl v0,s2,0xb
80026b40:  sll v0,v0,0x7
80026b44:  gte_ldVXY0 a0
80026b48:  gte_ldVXY1 v1
80026b4c:  gte_ldVXY2 v0
80026b50:  nNCCT
80026b5c:  li v0,-0x1
80026b60:  sw v0,0x10(sp)
80026b64:  move a0,s0
80026b68:  addiu a1,sp,0x18
80026b6c:  addiu a2,sp,0x20
80026b70:  jal 0x80025bc0
80026b74:  _addiu a3,sp,0x28
80026b78:  gte_ldVXY0 0x20(sp)
80026b7c:  gte_ldVZ0 0x24(sp)
80026b80:  gte_ldVXY1 0x28(sp)
80026b84:  gte_ldVZ1 0x2c(sp)
80026b88:  gte_ldVXY2 0x30(sp)
80026b8c:  gte_ldVZ2 0x34(sp)
80026b90:  nRTPT
80026b9c:  j 0x80026bb8
80026ba0:  _nop
80026ba4:  gte_ldVXY0 0x30(sp)
80026ba8:  gte_ldVZ0 0x34(sp)
80026bac:  nRTPS
80026bb8:  lhu v0,0x4(s1)
80026bbc:  addiu s0,s0,0x28
80026bc0:  sh v0,0xc(s0)
80026bc4:  lw v0,0x8(s1)
80026bc8:  nop
80026bcc:  sw v0,0x18(s0)
80026bd0:  lhu v0,0xc(s1)
80026bd4:  srl a0,s5,0xb
80026bd8:  sll a0,a0,0x7
80026bdc:  srl v1,s2,0xb
80026be0:  sll v1,v1,0x7
80026be4:  sh v0,0x24(s0)
80026be8:  srl v0,s4,0xb
80026bec:  sll v0,v0,0x7
80026bf0:  gte_ldVXY0 a0
80026bf4:  gte_ldVXY1 v1
80026bf8:  gte_ldVXY2 v0
80026bfc:  gte_stFLAG v0
80026c00:  nop
80026c04:  bltz v0,0x80026cd0
80026c08:  _nop
80026c0c:  NCLIP
80026c10:  gte_stMAC0 v0
80026c14:  nop
80026c18:  blez v0,0x80026cd0
80026c1c:  _nop
80026c20:  gte_stSZ0 v1
80026c24:  gte_stSZ1 a0
80026c28:  nop
80026c2c:  slt v0,a0,v1
80026c30:  beq v0,zero,0x80026c3c
80026c34:  _nop
80026c38:  move a0,v1
80026c3c:  gte_stSZ2 v1
80026c40:  nop
80026c44:  slt v0,v1,a0
80026c48:  beq v0,zero,0x80026c58
80026c4c:  _move a1,v1
80026c50:  move v1,a0
80026c54:  move a1,v1
80026c58:  gte_stIR0 v0
80026c5c:  nop
80026c60:  srl a2,v0,0x8
80026c64:  stsxy3_gt3 s0
80026c70:  nNCCT
80026c7c:  lhu v0,0x800(gp)
80026c80:  lui a0,0x1f80
80026c84:  lw a0,0x0(a0)
80026c88:  sll v1,a2,0x6
80026c8c:  addu v0,v0,v1
80026c90:  srl v1,a1,0x3
80026c94:  sll v1,v1,0x2
80026c98:  addu v1,v1,a0
80026c9c:  sh v0,0xe(s0)
80026ca0:  lw a0,0x0(v1)
80026ca4:  sll v0,s0,0x8
80026ca8:  srl v0,v0,0x8
80026cac:  sw v0,0x0(v1)
80026cb0:  lui v0,0x900
80026cb4:  or a0,a0,v0
80026cb8:  sw a0,0x0(s0)
80026cbc:  strgb3_gt3 s0
80026cc8:  j 0x800288d0
80026ccc:  _nop
80026cd0:  gte_stFLAG v0
80026cd4:  nop
80026cd8:  lui v1,0x2
80026cdc:  and v0,v0,v1
80026ce0:  beq v0,zero,0x800288d0
80026ce4:  _nop
80026ce8:  nNCCT
80026cf4:  sw zero,0x10(sp)
80026cf8:  move a0,s0
80026cfc:  addiu a1,sp,0x20
80026d00:  addiu a2,sp,0x28
80026d04:  jal 0x80025bc0
80026d08:  _addiu a3,sp,0x30
80026d0c:  j 0x800288d0
80026d10:  _nop
80026d14:  lui t0,0x1f80
80026d18:  lw t0,0x14(t0)
80026d1c:  gte_ldRGB 0x828(gp)
80026d20:  bne s6,zero,0x80026ec4
80026d24:  _addiu v1,s6,-0x1
80026d28:  gte_ldVXY0 0x28(sp)
80026d2c:  gte_ldVZ0 0x2c(sp)
80026d30:  nRTPS
80026d3c:  gte_stSXY2 0x8(t0)
80026d40:  srl v0,s2,0xb
80026d44:  sll v0,v0,0x7
80026d48:  gte_ldIR1 v0
80026d4c:  nCDP
80026d58:  gte_ldVXY0 0x18(sp)
80026d5c:  gte_ldVZ0 0x1c(sp)
80026d60:  nRTPS
80026d6c:  gte_stSXY2 0x10(t0)
80026d70:  srl v0,s3,0xb
80026d74:  sll v0,v0,0x7
80026d78:  gte_ldIR1 v0
80026d7c:  nCDP
80026d88:  gte_ldVXY0 0x30(sp)
80026d8c:  gte_ldVZ0 0x34(sp)
80026d90:  nRTPS
80026d9c:  gte_stSXY2 0x18(t0)
80026da0:  srl v0,s4,0xb
80026da4:  sll v0,v0,0x7
80026da8:  gte_ldIR1 v0
80026dac:  nCDP
80026db8:  nNCLIP
80026dc4:  gte_stMAC0 v0
80026dc8:  nop
80026dcc:  bgez v0,0x80026e24
80026dd0:  _nop
80026dd4:  strgb3_g3 t0
80026de0:  nAVSZ3
80026dec:  gte_stOTZ v1
80026df0:  lui v0,0x1f80
80026df4:  lw v0,0x0(v0)
80026df8:  srl v1,v1,0x1
80026dfc:  sll v1,v1,0x2
80026e00:  addu v1,v1,v0
80026e04:  lw a0,0x0(v1)
80026e08:  sll v0,t0,0x8
80026e0c:  srl v0,v0,0x8
80026e10:  sw v0,0x0(v1)
80026e14:  lui v0,0x600
80026e18:  or a0,a0,v0
80026e1c:  sw a0,0x0(t0)
80026e20:  addiu t0,t0,0x1c
80026e24:  gte_ldVXY0 0x20(sp)
80026e28:  gte_ldVZ0 0x24(sp)
80026e2c:  nRTPS
80026e38:  nNCLIP
80026e44:  gte_stMAC0 v0
80026e48:  nop
80026e4c:  blez v0,0x800288c8
80026e50:  _srl v0,s5,0xb
80026e54:  sll v0,v0,0x7
80026e58:  gte_ldIR1 v0
80026e5c:  nCDP
80026e68:  stsxy3_g3 t0
80026e74:  strgb3_g3 t0
80026e80:  nAVSZ3
80026e8c:  gte_stOTZ v1
80026e90:  lui v0,0x1f80
80026e94:  lw v0,0x0(v0)
80026e98:  srl v1,v1,0x1
80026e9c:  sll v1,v1,0x2
80026ea0:  addu v1,v1,v0
80026ea4:  lw a0,0x0(v1)
80026ea8:  sll v0,t0,0x8
80026eac:  srl v0,v0,0x8
80026eb0:  sw v0,0x0(v1)
80026eb4:  lui v0,0x600
80026eb8:  or a0,a0,v0
80026ebc:  j 0x800288c4
80026ec0:  _sw a0,0x0(t0)
80026ec4:  sltiu v0,v1,0xa
80026ec8:  beq v0,zero,0x800288c8
80026ecc:  _lui v0,0x8001
80026ed0:  addiu v0,v0,0x4f4
80026ed4:  sll v1,v1,0x2
80026ed8:  addu v1,v1,v0
80026edc:  lw v0,0x0(v1)
80026ee0:  nop
80026ee4:  jr v0
80026ee8:  _nop
80026eec:  srl a0,t6,0x1
80026ef0:  addu a0,s0,a0
80026ef4:  lui a1,0x8009
80026ef8:  addiu a1,a1,0x1120
80026efc:  srl v0,a0,0x6
80026f00:  sll v0,v0,0x2
80026f04:  srl v1,t7,0x6
80026f08:  sll v1,v1,0x7
80026f0c:  addu v0,v0,v1
80026f10:  addu v0,v0,a1
80026f14:  lw v1,0x80(v0)
80026f18:  andi a0,a0,0x3f
80026f1c:  sll a0,a0,0x1
80026f20:  andi v0,t7,0x3f
80026f24:  sll v0,v0,0x7
80026f28:  addu a0,a0,v0
80026f2c:  addu v1,v1,a0
80026f30:  lhu v1,0x0(v1)
80026f34:  srl v0,t2,0x1
80026f38:  addu v0,t5,v0
80026f3c:  sh t3,0x38(sp)
80026f40:  sh v0,0x3c(sp)
80026f44:  move t1,v1
80026f48:  andi v1,v1,0x7ff
80026f4c:  sll v1,v1,0x3
80026f50:  addu v1,t4,v1
80026f54:  sh v1,0x3a(sp)
80026f58:  gte_ldVXY0 0x18(sp)
80026f5c:  gte_ldVZ0 0x1c(sp)
80026f60:  nRTPS
80026f6c:  gte_stSXY2 0x8(t0)
80026f70:  srl v0,s3,0xb
80026f74:  sll v0,v0,0x7
80026f78:  gte_ldIR1 v0
80026f7c:  nCDP
80026f88:  gte_ldVXY0 0x20(sp)
80026f8c:  gte_ldVZ0 0x24(sp)
80026f90:  nRTPS
80026f9c:  gte_stSXY2 0x10(t0)
80026fa0:  srl v0,s5,0xb
80026fa4:  sll v0,v0,0x7
80026fa8:  gte_ldIR1 v0
80026fac:  nCDP
80026fb8:  gte_ldVXY0 0x38(sp)
80026fbc:  gte_ldVZ0 0x3c(sp)
80026fc0:  nRTPS
80026fcc:  gte_stSXY2 0x18(t0)
80026fd0:  srl v0,t1,0xb
80026fd4:  sll v0,v0,0x7
80026fd8:  gte_ldIR1 v0
80026fdc:  nCDP
80026fe8:  nNCLIP
80026ff4:  gte_stMAC0 v0
80026ff8:  nop
80026ffc:  bgez v0,0x80027054
80027000:  _nop
80027004:  strgb3_g3 t0
80027010:  nAVSZ3
8002701c:  gte_stOTZ v1
80027020:  lui v0,0x1f80
80027024:  lw v0,0x0(v0)
80027028:  srl v1,v1,0x1
8002702c:  sll v1,v1,0x2
80027030:  addu v1,v1,v0
80027034:  lw a0,0x0(v1)
80027038:  sll v0,t0,0x8
8002703c:  srl v0,v0,0x8
80027040:  sw v0,0x0(v1)
80027044:  lui v0,0x600
80027048:  or a0,a0,v0
8002704c:  sw a0,0x0(t0)
80027050:  addiu t0,t0,0x1c
80027054:  gte_ldVXY0 0x30(sp)
80027058:  gte_ldVZ0 0x34(sp)
8002705c:  nRTPS
80027068:  gte_stOTZ v0
8002706c:  srl v0,s4,0xb
80027070:  sll v0,v0,0x7
80027074:  gte_ldIR1 v0
80027078:  nCDP
80027084:  nNCLIP
80027090:  gte_stMAC0 v0
80027094:  nop
80027098:  blez v0,0x800270fc
8002709c:  _nop
800270a0:  stsxy3_g3 t0
800270ac:  strgb3_g3 t0
800270b8:  nAVSZ3
800270c4:  gte_stOTZ v1
800270c8:  lui v0,0x1f80
800270cc:  lw v0,0x0(v0)
800270d0:  srl v1,v1,0x1
800270d4:  sll v1,v1,0x2
800270d8:  addu v1,v1,v0
800270dc:  lw a0,0x0(v1)
800270e0:  sll v0,t0,0x8
800270e4:  srl v0,v0,0x8
800270e8:  sw v0,0x0(v1)
800270ec:  lui v0,0x600
800270f0:  or a0,a0,v0
800270f4:  sw a0,0x0(t0)
800270f8:  addiu t0,t0,0x1c
800270fc:  gte_ldVXY0 0x28(sp)
80027100:  gte_ldVZ0 0x2c(sp)
80027104:  nRTPS
80027110:  nNCLIP
8002711c:  gte_stMAC0 v0
80027120:  nop
80027124:  bgez v0,0x800288c8
80027128:  _nop
8002712c:  nAVSZ3
80027138:  gte_stOTZ a1
8002713c:  srl v0,s2,0xb
80027140:  sll v0,v0,0x7
80027144:  gte_ldIR1 v0
80027148:  nCDP
80027154:  lui v0,0x1f80
80027158:  lw v0,0x0(v0)
8002715c:  srl v1,a1,0x1
80027160:  sll v1,v1,0x2
80027164:  addu v1,v1,v0
80027168:  lw a0,0x0(v1)
8002716c:  sll v0,t0,0x8
80027170:  srl v0,v0,0x8
80027174:  sw v0,0x0(v1)
80027178:  lui v0,0x600
8002717c:  or a0,a0,v0
80027180:  sw a0,0x0(t0)
80027184:  stsxy3_g3 t0
80027190:  strgb3_g3 t0
8002719c:  j 0x800288c8
800271a0:  _addiu t0,t0,0x1c
800271a4:  srl a0,t6,0x1
800271a8:  addu a2,t7,t6
800271ac:  addu a0,s0,a0
800271b0:  lui a1,0x8009
800271b4:  addiu a1,a1,0x1120
800271b8:  srl v0,a0,0x6
800271bc:  sll v0,v0,0x2
800271c0:  srl v1,a2,0x6
800271c4:  sll v1,v1,0x7
800271c8:  addu v0,v0,v1
800271cc:  addu v0,v0,a1
800271d0:  lw v0,0x80(v0)
800271d4:  andi a0,a0,0x3f
800271d8:  sll a0,a0,0x1
800271dc:  andi a2,a2,0x3f
800271e0:  sll a2,a2,0x7
800271e4:  addu a0,a0,a2
800271e8:  addu v0,v0,a0
800271ec:  lhu v1,0x0(v0)
800271f0:  addu v0,t3,t2
800271f4:  sh v0,0x38(sp)
800271f8:  srl v0,t2,0x1
800271fc:  addu v0,t5,v0
80027200:  sh v0,0x3c(sp)
80027204:  move t1,v1
80027208:  andi v1,v1,0x7ff
8002720c:  sll v1,v1,0x3
80027210:  addu v1,t4,v1
80027214:  sh v1,0x3a(sp)
80027218:  gte_ldVXY0 0x20(sp)
8002721c:  gte_ldVZ0 0x24(sp)
80027220:  nRTPS
8002722c:  gte_stSXY2 0x8(t0)
80027230:  srl v0,s5,0xb
80027234:  sll v0,v0,0x7
80027238:  gte_ldIR1 v0
8002723c:  nCDP
80027248:  gte_ldVXY0 0x18(sp)
8002724c:  gte_ldVZ0 0x1c(sp)
80027250:  nRTPS
8002725c:  gte_stSXY2 0x10(t0)
80027260:  srl v0,s3,0xb
80027264:  sll v0,v0,0x7
80027268:  gte_ldIR1 v0
8002726c:  nCDP
80027278:  gte_ldVXY0 0x38(sp)
8002727c:  gte_ldVZ0 0x3c(sp)
80027280:  nRTPS
8002728c:  gte_stSXY2 0x18(t0)
80027290:  srl v0,t1,0xb
80027294:  sll v0,v0,0x7
80027298:  gte_ldIR1 v0
8002729c:  nCDP
800272a8:  nNCLIP
800272b4:  gte_stMAC0 v0
800272b8:  nop
800272bc:  blez v0,0x80027314
800272c0:  _nop
800272c4:  strgb3_g3 t0
800272d0:  nAVSZ3
800272dc:  gte_stOTZ v1
800272e0:  lui v0,0x1f80
800272e4:  lw v0,0x0(v0)
800272e8:  srl v1,v1,0x1
800272ec:  sll v1,v1,0x2
800272f0:  addu v1,v1,v0
800272f4:  lw a0,0x0(v1)
800272f8:  sll v0,t0,0x8
800272fc:  srl v0,v0,0x8
80027300:  sw v0,0x0(v1)
80027304:  lui v0,0x600
80027308:  or a0,a0,v0
8002730c:  sw a0,0x0(t0)
80027310:  addiu t0,t0,0x1c
80027314:  gte_ldVXY0 0x28(sp)
80027318:  gte_ldVZ0 0x2c(sp)
8002731c:  nRTPS
80027328:  gte_stOTZ v0
8002732c:  srl v0,s2,0xb
80027330:  sll v0,v0,0x7
80027334:  gte_ldIR1 v0
80027338:  nCDP
80027344:  nNCLIP
80027350:  gte_stMAC0 v0
80027354:  nop
80027358:  bgez v0,0x800273bc
8002735c:  _nop
80027360:  stsxy3_g3 t0
8002736c:  strgb3_g3 t0
80027378:  nAVSZ3
80027384:  gte_stOTZ v1
80027388:  lui v0,0x1f80
8002738c:  lw v0,0x0(v0)
80027390:  srl v1,v1,0x1
80027394:  sll v1,v1,0x2
80027398:  addu v1,v1,v0
8002739c:  lw a0,0x0(v1)
800273a0:  sll v0,t0,0x8
800273a4:  srl v0,v0,0x8
800273a8:  sw v0,0x0(v1)
800273ac:  lui v0,0x600
800273b0:  or a0,a0,v0
800273b4:  sw a0,0x0(t0)
800273b8:  addiu t0,t0,0x1c
800273bc:  gte_ldVXY0 0x30(sp)
800273c0:  gte_ldVZ0 0x34(sp)
800273c4:  nRTPS
800273d0:  nNCLIP
800273dc:  gte_stMAC0 v0
800273e0:  nop
800273e4:  blez v0,0x800288c8
800273e8:  _nop
800273ec:  nAVSZ3
800273f8:  gte_stOTZ a1
800273fc:  srl v0,s4,0xb
80027400:  sll v0,v0,0x7
80027404:  gte_ldIR1 v0
80027408:  nCDP
80027414:  lui v0,0x1f80
80027418:  lw v0,0x0(v0)
8002741c:  srl v1,a1,0x1
80027420:  sll v1,v1,0x2
80027424:  addu v1,v1,v0
80027428:  lw a0,0x0(v1)
8002742c:  sll v0,t0,0x8
80027430:  srl v0,v0,0x8
80027434:  sw v0,0x0(v1)
80027438:  lui v0,0x600
8002743c:  or a0,a0,v0
80027440:  sw a0,0x0(t0)
80027444:  stsxy3_g3 t0
80027450:  strgb3_g3 t0
8002745c:  j 0x800288c8
80027460:  _addiu t0,t0,0x1c
80027464:  srl a1,t6,0x1
80027468:  addu a1,t7,a1
8002746c:  addu a0,s0,t6
80027470:  lui a2,0x8009
80027474:  addiu a2,a2,0x1120
80027478:  srl v0,a0,0x6
8002747c:  sll v0,v0,0x2
80027480:  srl v1,a1,0x6
80027484:  sll v1,v1,0x7
80027488:  addu v0,v0,v1
8002748c:  addu v0,v0,a2
80027490:  lw v0,0x80(v0)
80027494:  andi a0,a0,0x3f
80027498:  sll a0,a0,0x1
8002749c:  andi a1,a1,0x3f
800274a0:  sll a1,a1,0x7
800274a4:  addu a0,a0,a1
800274a8:  addu v0,v0,a0
800274ac:  lhu v1,0x0(v0)
800274b0:  srl v0,t2,0x1
800274b4:  addu v0,t3,v0
800274b8:  sh v0,0x38(sp)
800274bc:  addu v0,t5,t2
800274c0:  sh v0,0x3c(sp)
800274c4:  move t1,v1
800274c8:  andi v1,v1,0x7ff
800274cc:  sll v1,v1,0x3
800274d0:  addu v1,t4,v1
800274d4:  sh v1,0x3a(sp)
800274d8:  gte_ldVXY0 0x28(sp)
800274dc:  gte_ldVZ0 0x2c(sp)
800274e0:  nRTPS
800274ec:  gte_stSXY2 0x8(t0)
800274f0:  srl v0,s2,0xb
800274f4:  sll v0,v0,0x7
800274f8:  gte_ldIR1 v0
800274fc:  nCDP
80027508:  gte_ldVXY0 0x18(sp)
8002750c:  gte_ldVZ0 0x1c(sp)
80027510:  nRTPS
8002751c:  gte_stSXY2 0x10(t0)
80027520:  srl v0,s3,0xb
80027524:  sll v0,v0,0x7
80027528:  gte_ldIR1 v0
8002752c:  nCDP
80027538:  gte_ldVXY0 0x38(sp)
8002753c:  gte_ldVZ0 0x3c(sp)
80027540:  nRTPS
8002754c:  gte_stSXY2 0x18(t0)
80027550:  srl v0,t1,0xb
80027554:  sll v0,v0,0x7
80027558:  gte_ldIR1 v0
8002755c:  nCDP
80027568:  nNCLIP
80027574:  gte_stMAC0 v0
80027578:  nop
8002757c:  bgez v0,0x800275d4
80027580:  _nop
80027584:  strgb3_g3 t0
80027590:  nAVSZ3
8002759c:  gte_stOTZ v1
800275a0:  lui v0,0x1f80
800275a4:  lw v0,0x0(v0)
800275a8:  srl v1,v1,0x1
800275ac:  sll v1,v1,0x2
800275b0:  addu v1,v1,v0
800275b4:  lw a0,0x0(v1)
800275b8:  sll v0,t0,0x8
800275bc:  srl v0,v0,0x8
800275c0:  sw v0,0x0(v1)
800275c4:  lui v0,0x600
800275c8:  or a0,a0,v0
800275cc:  sw a0,0x0(t0)
800275d0:  addiu t0,t0,0x1c
800275d4:  gte_ldVXY0 0x20(sp)
800275d8:  gte_ldVZ0 0x24(sp)
800275dc:  nRTPS
800275e8:  gte_stOTZ v0
800275ec:  srl v0,s5,0xb
800275f0:  sll v0,v0,0x7
800275f4:  gte_ldIR1 v0
800275f8:  nCDP
80027604:  nNCLIP
80027610:  gte_stMAC0 v0
80027614:  nop
80027618:  blez v0,0x8002767c
8002761c:  _nop
80027620:  stsxy3_g3 t0
8002762c:  strgb3_g3 t0
80027638:  nAVSZ3
80027644:  gte_stOTZ v1
80027648:  lui v0,0x1f80
8002764c:  lw v0,0x0(v0)
80027650:  srl v1,v1,0x1
80027654:  sll v1,v1,0x2
80027658:  addu v1,v1,v0
8002765c:  lw a0,0x0(v1)
80027660:  sll v0,t0,0x8
80027664:  srl v0,v0,0x8
80027668:  sw v0,0x0(v1)
8002766c:  lui v0,0x600
80027670:  or a0,a0,v0
80027674:  sw a0,0x0(t0)
80027678:  addiu t0,t0,0x1c
8002767c:  gte_ldVXY0 0x30(sp)
80027680:  gte_ldVZ0 0x34(sp)
80027684:  nRTPS
80027690:  nNCLIP
8002769c:  gte_stMAC0 v0
800276a0:  nop
800276a4:  bgez v0,0x800288c8
800276a8:  _nop
800276ac:  nAVSZ3
800276b8:  gte_stOTZ a1
800276bc:  srl v0,s4,0xb
800276c0:  sll v0,v0,0x7
800276c4:  gte_ldIR1 v0
800276c8:  nCDP
800276d4:  lui v0,0x1f80
800276d8:  lw v0,0x0(v0)
800276dc:  srl v1,a1,0x1
800276e0:  sll v1,v1,0x2
800276e4:  addu v1,v1,v0
800276e8:  lw a0,0x0(v1)
800276ec:  sll v0,t0,0x8
800276f0:  srl v0,v0,0x8
800276f4:  sw v0,0x0(v1)
800276f8:  lui v0,0x600
800276fc:  or a0,a0,v0
80027700:  sw a0,0x0(t0)
80027704:  stsxy3_g3 t0
80027710:  strgb3_g3 t0
8002771c:  j 0x800288c8
80027720:  _addiu t0,t0,0x1c
80027724:  srl a0,t6,0x1
80027728:  addu a0,t7,a0
8002772c:  lui a1,0x8009
80027730:  addiu a1,a1,0x1120
80027734:  srl v0,s0,0x6
80027738:  sll v0,v0,0x2
8002773c:  srl v1,a0,0x6
80027740:  sll v1,v1,0x7
80027744:  addu v0,v0,v1
80027748:  addu v0,v0,a1
8002774c:  lw v1,0x80(v0)
80027750:  andi v0,s0,0x3f
80027754:  sll v0,v0,0x1
80027758:  andi a0,a0,0x3f
8002775c:  sll a0,a0,0x7
80027760:  addu v0,v0,a0
80027764:  addu v1,v1,v0
80027768:  lhu v1,0x0(v1)
8002776c:  srl v0,t2,0x1
80027770:  addu v0,t3,v0
80027774:  sh v0,0x38(sp)
80027778:  sh t5,0x3c(sp)
8002777c:  move t1,v1
80027780:  andi v1,v1,0x7ff
80027784:  sll v1,v1,0x3
80027788:  addu v1,t4,v1
8002778c:  sh v1,0x3a(sp)
80027790:  gte_ldVXY0 0x18(sp)
80027794:  gte_ldVZ0 0x1c(sp)
80027798:  nRTPS
800277a4:  gte_stSXY2 0x8(t0)
800277a8:  srl v0,s3,0xb
800277ac:  sll v0,v0,0x7
800277b0:  gte_ldIR1 v0
800277b4:  nCDP
800277c0:  gte_ldVXY0 0x28(sp)
800277c4:  gte_ldVZ0 0x2c(sp)
800277c8:  nRTPS
800277d4:  gte_stSXY2 0x10(t0)
800277d8:  srl v0,s2,0xb
800277dc:  sll v0,v0,0x7
800277e0:  gte_ldIR1 v0
800277e4:  nCDP
800277f0:  gte_ldVXY0 0x38(sp)
800277f4:  gte_ldVZ0 0x3c(sp)
800277f8:  nRTPS
80027804:  gte_stSXY2 0x18(t0)
80027808:  srl v0,t1,0xb
8002780c:  sll v0,v0,0x7
80027810:  gte_ldIR1 v0
80027814:  nCDP
80027820:  nNCLIP
8002782c:  gte_stMAC0 v0
80027830:  nop
80027834:  blez v0,0x8002788c
80027838:  _nop
8002783c:  strgb3_g3 t0
80027848:  nAVSZ3
80027854:  gte_stOTZ v1
80027858:  lui v0,0x1f80
8002785c:  lw v0,0x0(v0)
80027860:  srl v1,v1,0x1
80027864:  sll v1,v1,0x2
80027868:  addu v1,v1,v0
8002786c:  lw a0,0x0(v1)
80027870:  sll v0,t0,0x8
80027874:  srl v0,v0,0x8
80027878:  sw v0,0x0(v1)
8002787c:  lui v0,0x600
80027880:  or a0,a0,v0
80027884:  sw a0,0x0(t0)
80027888:  addiu t0,t0,0x1c
8002788c:  gte_ldVXY0 0x30(sp)
80027890:  gte_ldVZ0 0x34(sp)
80027894:  nRTPS
800278a0:  gte_stOTZ v0
800278a4:  srl v0,s4,0xb
800278a8:  sll v0,v0,0x7
800278ac:  gte_ldIR1 v0
800278b0:  nCDP
800278bc:  nNCLIP
800278c8:  gte_stMAC0 v0
800278cc:  nop
800278d0:  bgez v0,0x80027934
800278d4:  _nop
800278d8:  stsxy3_g3 t0
800278e4:  strgb3_g3 t0
800278f0:  nAVSZ3
800278fc:  gte_stOTZ v1
80027900:  lui v0,0x1f80
80027904:  lw v0,0x0(v0)
80027908:  srl v1,v1,0x1
8002790c:  sll v1,v1,0x2
80027910:  addu v1,v1,v0
80027914:  lw a0,0x0(v1)
80027918:  sll v0,t0,0x8
8002791c:  srl v0,v0,0x8
80027920:  sw v0,0x0(v1)
80027924:  lui v0,0x600
80027928:  or a0,a0,v0
8002792c:  sw a0,0x0(t0)
80027930:  addiu t0,t0,0x1c
80027934:  gte_ldVXY0 0x20(sp)
80027938:  gte_ldVZ0 0x24(sp)
8002793c:  nRTPS
80027948:  nNCLIP
80027954:  gte_stMAC0 v0
80027958:  nop
8002795c:  blez v0,0x800288c8
80027960:  _nop
80027964:  nAVSZ3
80027970:  gte_stOTZ a1
80027974:  srl v0,s5,0xb
80027978:  sll v0,v0,0x7
8002797c:  gte_ldIR1 v0
80027980:  nCDP
8002798c:  lui v0,0x1f80
80027990:  lw v0,0x0(v0)
80027994:  srl v1,a1,0x1
80027998:  sll v1,v1,0x2
8002799c:  addu v1,v1,v0
800279a0:  lw a0,0x0(v1)
800279a4:  sll v0,t0,0x8
800279a8:  srl v0,v0,0x8
800279ac:  sw v0,0x0(v1)
800279b0:  lui v0,0x600
800279b4:  or a0,a0,v0
800279b8:  sw a0,0x0(t0)
800279bc:  stsxy3_g3 t0
800279c8:  strgb3_g3 t0
800279d4:  j 0x800288c8
800279d8:  _addiu t0,t0,0x1c
800279dc:  srl a1,t6,0x1
800279e0:  addu a0,s0,a1
800279e4:  lui a3,0x8009
800279e8:  addiu a3,a3,0x1120
800279ec:  srl v0,a0,0x6
800279f0:  sll v0,v0,0x2
800279f4:  srl v1,t7,0x6
800279f8:  sll v1,v1,0x7
800279fc:  addu v0,v0,v1
80027a00:  addu v0,v0,a3
80027a04:  lw v1,0x80(v0)
80027a08:  andi a0,a0,0x3f
80027a0c:  sll a0,a0,0x1
80027a10:  andi v0,t7,0x3f
80027a14:  sll v0,v0,0x7
80027a18:  addu a0,a0,v0
80027a1c:  addu v1,v1,a0
80027a20:  lhu v0,0x0(v1)
80027a24:  srl a2,t2,0x1
80027a28:  addu v1,t5,a2
80027a2c:  sh t3,0x38(sp)
80027a30:  sh v1,0x3c(sp)
80027a34:  move t1,v0
80027a38:  andi v0,v0,0x7ff
80027a3c:  sll v0,v0,0x3
80027a40:  addu v0,t4,v0
80027a44:  sh v0,0x3a(sp)
80027a48:  gte_ldVXY0 0x18(sp)
80027a4c:  gte_ldVZ0 0x1c(sp)
80027a50:  nRTPS
80027a5c:  gte_stSXY2 0x8(t0)
80027a60:  srl v0,s3,0xb
80027a64:  sll v0,v0,0x7
80027a68:  gte_ldIR1 v0
80027a6c:  nCDP
80027a78:  gte_ldVXY0 0x38(sp)
80027a7c:  gte_ldVZ0 0x3c(sp)
80027a80:  nRTPS
80027a8c:  gte_stSXY2 0x10(t0)
80027a90:  srl v0,t1,0xb
80027a94:  sll v0,v0,0x7
80027a98:  gte_ldIR1 v0
80027a9c:  nCDP
80027aa8:  gte_ldVXY0 0x20(sp)
80027aac:  gte_ldVZ0 0x24(sp)
80027ab0:  nRTPS
80027abc:  gte_stSXY2 0x18(t0)
80027ac0:  srl v0,s5,0xb
80027ac4:  sll v0,v0,0x7
80027ac8:  gte_ldIR1 v0
80027acc:  nCDP
80027ad8:  addu a1,t7,a1
80027adc:  addu a0,s0,t6
80027ae0:  srl v0,a0,0x6
80027ae4:  sll v0,v0,0x2
80027ae8:  srl v1,a1,0x6
80027aec:  sll v1,v1,0x7
80027af0:  addu v0,v0,v1
80027af4:  addu v0,v0,a3
80027af8:  lw v0,0x80(v0)
80027afc:  andi a0,a0,0x3f
80027b00:  sll a0,a0,0x1
80027b04:  andi a1,a1,0x3f
80027b08:  sll a1,a1,0x7
80027b0c:  addu a0,a0,a1
80027b10:  addu v0,v0,a0
80027b14:  lhu v0,0x0(v0)
80027b18:  addu a2,t3,a2
80027b1c:  addu v1,t5,t2
80027b20:  sh a2,0x40(sp)
80027b24:  sh v1,0x44(sp)
80027b28:  move a1,v0
80027b2c:  andi v0,v0,0x7ff
80027b30:  sll v0,v0,0x3
80027b34:  addu v0,t4,v0
80027b38:  sh v0,0x42(sp)
80027b3c:  NCLIP
80027b40:  gte_stMAC0 v0
80027b44:  nop
80027b48:  blez v0,0x80027ba0
80027b4c:  _nop
80027b50:  strgb3_g3 t0
80027b5c:  nAVSZ3
80027b68:  gte_stOTZ v1
80027b6c:  lui v0,0x1f80
80027b70:  lw v0,0x0(v0)
80027b74:  srl v1,v1,0x1
80027b78:  sll v1,v1,0x2
80027b7c:  addu v1,v1,v0
80027b80:  lw a0,0x0(v1)
80027b84:  sll v0,t0,0x8
80027b88:  srl v0,v0,0x8
80027b8c:  sw v0,0x0(v1)
80027b90:  lui v0,0x600
80027b94:  or a0,a0,v0
80027b98:  sw a0,0x0(t0)
80027b9c:  addiu t0,t0,0x1c
80027ba0:  gte_ldVXY0 0x40(sp)
80027ba4:  gte_ldVZ0 0x44(sp)
80027ba8:  nRTPS
80027bb4:  gte_stOTZ v0
80027bb8:  srl v0,a1,0xb
80027bbc:  sll v0,v0,0x7
80027bc0:  gte_ldIR1 v0
80027bc4:  nCDP
80027bd0:  gte_stRGB0 0x3c(t0)
80027bd4:  gte_stSXY0 0x40(t0)
80027bd8:  gte_stRGB2 0x44(t0)
80027bdc:  gte_stSXY2 0x48(t0)
80027be0:  nNCLIP
80027bec:  gte_stMAC0 v0
80027bf0:  nop
80027bf4:  bgez v0,0x80027c54
80027bf8:  _nop
80027bfc:  stsxy3_g3 t0
80027c08:  strgb3_g3 t0
80027c14:  nAVSZ3
80027c20:  gte_stOTZ v1
80027c24:  lui v0,0x1f80
80027c28:  lw v0,0x0(v0)
80027c2c:  srl v1,v1,0x1
80027c30:  sll v1,v1,0x2
80027c34:  addu v1,v1,v0
80027c38:  lw a0,0x0(v1)
80027c3c:  sll v0,t0,0x8
80027c40:  srl v0,v0,0x8
80027c44:  sw v0,0x0(v1)
80027c48:  lui v0,0x600
80027c4c:  or a0,a0,v0
80027c50:  sw a0,0x0(t0)
80027c54:  gte_ldVXY0 0x30(sp)
80027c58:  gte_ldVZ0 0x34(sp)
80027c5c:  nRTPS
80027c68:  gte_stOTZ v0
80027c6c:  srl v0,s4,0xb
80027c70:  sll v0,v0,0x7
80027c74:  gte_ldIR1 v0
80027c78:  nCDP
80027c84:  addiu t0,t0,0x1c
80027c88:  nop
80027c8c:  NCLIP
80027c90:  gte_stMAC0 v0
80027c94:  nop
80027c98:  blez v0,0x80027cf8
80027c9c:  _nop
80027ca0:  stsxy3_g3 t0
80027cac:  strgb3_g3 t0
80027cb8:  nAVSZ3
80027cc4:  gte_stOTZ v1
80027cc8:  lui v0,0x1f80
80027ccc:  lw v0,0x0(v0)
80027cd0:  srl v1,v1,0x1
80027cd4:  sll v1,v1,0x2
80027cd8:  addu v1,v1,v0
80027cdc:  lw a0,0x0(v1)
80027ce0:  sll v0,t0,0x8
80027ce4:  srl v0,v0,0x8
80027ce8:  sw v0,0x0(v1)
80027cec:  lui v0,0x600
80027cf0:  or a0,a0,v0
80027cf4:  sw a0,0x0(t0)
80027cf8:  gte_ldVXY0 0x28(sp)
80027cfc:  gte_ldVZ0 0x2c(sp)
80027d00:  nRTPS
80027d0c:  gte_stSXY2 0x34(t0)
80027d10:  gte_ldSXY0 0x24(t0)
80027d14:  gte_ldSXY1 0x2c(t0)
80027d18:  addiu t0,t0,0x1c
80027d1c:  nop
80027d20:  NCLIP
80027d24:  gte_stMAC0 v0
80027d28:  nop
80027d2c:  bgez v0,0x800288c8
80027d30:  _nop
80027d34:  nAVSZ3
80027d40:  gte_stOTZ a1
80027d44:  srl v0,s2,0xb
80027d48:  sll v0,v0,0x7
80027d4c:  gte_ldIR1 v0
80027d50:  nCDP
80027d5c:  lui v0,0x1f80
80027d60:  lw v0,0x0(v0)
80027d64:  srl v1,a1,0x1
80027d68:  sll v1,v1,0x2
80027d6c:  addu v1,v1,v0
80027d70:  lw a0,0x0(v1)
80027d74:  sll v0,t0,0x8
80027d78:  srl v0,v0,0x8
80027d7c:  sw v0,0x0(v1)
80027d80:  lui v0,0x600
80027d84:  or a0,a0,v0
80027d88:  sw a0,0x0(t0)
80027d8c:  gte_stRGB2 0x14(t0)
80027d90:  j 0x800288c8
80027d94:  _addiu t0,t0,0x1c
80027d98:  srl a2,t6,0x1
80027d9c:  addu a1,t7,t6
80027da0:  addu a0,s0,a2
80027da4:  lui a3,0x8009
80027da8:  addiu a3,a3,0x1120
80027dac:  srl v0,a0,0x6
80027db0:  sll v0,v0,0x2
80027db4:  srl v1,a1,0x6
80027db8:  sll v1,v1,0x7
80027dbc:  addu v0,v0,v1
80027dc0:  addu v0,v0,a3
80027dc4:  lw v0,0x80(v0)
80027dc8:  andi a0,a0,0x3f
80027dcc:  sll a0,a0,0x1
80027dd0:  andi a1,a1,0x3f
80027dd4:  sll a1,a1,0x7
80027dd8:  addu a0,a0,a1
80027ddc:  addu v0,v0,a0
80027de0:  lhu v0,0x0(v0)
80027de4:  addu v1,t3,t2
80027de8:  srl a1,t2,0x1
80027dec:  sh v1,0x38(sp)
80027df0:  addu v1,t5,a1
80027df4:  sh v1,0x3c(sp)
80027df8:  move t1,v0
80027dfc:  andi v0,v0,0x7ff
80027e00:  sll v0,v0,0x3
80027e04:  addu v0,t4,v0
80027e08:  sh v0,0x3a(sp)
80027e0c:  gte_ldVXY0 0x20(sp)
80027e10:  gte_ldVZ0 0x24(sp)
80027e14:  nRTPS
80027e20:  gte_stSXY2 0x8(t0)
80027e24:  srl v0,s5,0xb
80027e28:  sll v0,v0,0x7
80027e2c:  gte_ldIR1 v0
80027e30:  nCDP
80027e3c:  gte_ldVXY0 0x38(sp)
80027e40:  gte_ldVZ0 0x3c(sp)
80027e44:  nRTPS
80027e50:  gte_stSXY2 0x10(t0)
80027e54:  srl v0,t1,0xb
80027e58:  sll v0,v0,0x7
80027e5c:  gte_ldIR1 v0
80027e60:  nCDP
80027e6c:  gte_ldVXY0 0x18(sp)
80027e70:  gte_ldVZ0 0x1c(sp)
80027e74:  nRTPS
80027e80:  gte_stSXY2 0x18(t0)
80027e84:  srl v0,s3,0xb
80027e88:  sll v0,v0,0x7
80027e8c:  gte_ldIR1 v0
80027e90:  nCDP
80027e9c:  addu a2,t7,a2
80027ea0:  addu a0,s0,t6
80027ea4:  srl v0,a0,0x6
80027ea8:  sll v0,v0,0x2
80027eac:  srl v1,a2,0x6
80027eb0:  sll v1,v1,0x7
80027eb4:  addu v0,v0,v1
80027eb8:  addu v0,v0,a3
80027ebc:  lw v0,0x80(v0)
80027ec0:  andi a0,a0,0x3f
80027ec4:  sll a0,a0,0x1
80027ec8:  andi a2,a2,0x3f
80027ecc:  sll a2,a2,0x7
80027ed0:  addu a0,a0,a2
80027ed4:  addu v0,v0,a0
80027ed8:  lhu v0,0x0(v0)
80027edc:  addu a1,t3,a1
80027ee0:  addu v1,t5,t2
80027ee4:  sh a1,0x40(sp)
80027ee8:  sh v1,0x44(sp)
80027eec:  move a1,v0
80027ef0:  andi v0,v0,0x7ff
80027ef4:  sll v0,v0,0x3
80027ef8:  addu v0,t4,v0
80027efc:  sh v0,0x42(sp)
80027f00:  NCLIP
80027f04:  gte_stMAC0 v0
80027f08:  nop
80027f0c:  bgez v0,0x80027f64
80027f10:  _nop
80027f14:  strgb3_g3 t0
80027f20:  nAVSZ3
80027f2c:  gte_stOTZ v1
80027f30:  lui v0,0x1f80
80027f34:  lw v0,0x0(v0)
80027f38:  srl v1,v1,0x1
80027f3c:  sll v1,v1,0x2
80027f40:  addu v1,v1,v0
80027f44:  lw a0,0x0(v1)
80027f48:  sll v0,t0,0x8
80027f4c:  srl v0,v0,0x8
80027f50:  sw v0,0x0(v1)
80027f54:  lui v0,0x600
80027f58:  or a0,a0,v0
80027f5c:  sw a0,0x0(t0)
80027f60:  addiu t0,t0,0x1c
80027f64:  gte_ldVXY0 0x40(sp)
80027f68:  gte_ldVZ0 0x44(sp)
80027f6c:  nRTPS
80027f78:  gte_stOTZ v0
80027f7c:  srl v0,a1,0xb
80027f80:  sll v0,v0,0x7
80027f84:  gte_ldIR1 v0
80027f88:  nCDP
80027f94:  gte_stRGB0 0x3c(t0)
80027f98:  gte_stSXY0 0x40(t0)
80027f9c:  gte_stRGB2 0x44(t0)
80027fa0:  gte_stSXY2 0x48(t0)
80027fa4:  nNCLIP
80027fb0:  gte_stMAC0 v0
80027fb4:  nop
80027fb8:  blez v0,0x80028018
80027fbc:  _nop
80027fc0:  stsxy3_g3 t0
80027fcc:  strgb3_g3 t0
80027fd8:  nAVSZ3
80027fe4:  gte_stOTZ v1
80027fe8:  lui v0,0x1f80
80027fec:  lw v0,0x0(v0)
80027ff0:  srl v1,v1,0x1
80027ff4:  sll v1,v1,0x2
80027ff8:  addu v1,v1,v0
80027ffc:  lw a0,0x0(v1)
80028000:  sll v0,t0,0x8
80028004:  srl v0,v0,0x8
80028008:  sw v0,0x0(v1)
8002800c:  lui v0,0x600
80028010:  or a0,a0,v0
80028014:  sw a0,0x0(t0)
80028018:  gte_ldVXY0 0x28(sp)
8002801c:  gte_ldVZ0 0x2c(sp)
80028020:  nRTPS
8002802c:  gte_stOTZ v0
80028030:  srl v0,s2,0xb
80028034:  sll v0,v0,0x7
80028038:  gte_ldIR1 v0
8002803c:  nCDP
80028048:  addiu t0,t0,0x1c
8002804c:  nop
80028050:  NCLIP
80028054:  gte_stMAC0 v0
80028058:  nop
8002805c:  bgez v0,0x800280bc
80028060:  _nop
80028064:  stsxy3_g3 t0
80028070:  strgb3_g3 t0
8002807c:  nAVSZ3
80028088:  gte_stOTZ v1
8002808c:  lui v0,0x1f80
80028090:  lw v0,0x0(v0)
80028094:  srl v1,v1,0x1
80028098:  sll v1,v1,0x2
8002809c:  addu v1,v1,v0
800280a0:  lw a0,0x0(v1)
800280a4:  sll v0,t0,0x8
800280a8:  srl v0,v0,0x8
800280ac:  sw v0,0x0(v1)
800280b0:  lui v0,0x600
800280b4:  or a0,a0,v0
800280b8:  sw a0,0x0(t0)
800280bc:  gte_ldVXY0 0x30(sp)
800280c0:  gte_ldVZ0 0x34(sp)
800280c4:  nRTPS
800280d0:  gte_stSXY2 0x34(t0)
800280d4:  gte_ldSXY0 0x24(t0)
800280d8:  gte_ldSXY1 0x2c(t0)
800280dc:  addiu t0,t0,0x1c
800280e0:  nop
800280e4:  NCLIP
800280e8:  gte_stMAC0 v0
800280ec:  nop
800280f0:  blez v0,0x800288c8
800280f4:  _nop
800280f8:  nAVSZ3
80028104:  gte_stOTZ a1
80028108:  srl v0,s4,0xb
8002810c:  sll v0,v0,0x7
80028110:  gte_ldIR1 v0
80028114:  nCDP
80028120:  lui v0,0x1f80
80028124:  lw v0,0x0(v0)
80028128:  srl v1,a1,0x1
8002812c:  sll v1,v1,0x2
80028130:  addu v1,v1,v0
80028134:  lw a0,0x0(v1)
80028138:  sll v0,t0,0x8
8002813c:  srl v0,v0,0x8
80028140:  sw v0,0x0(v1)
80028144:  lui v0,0x600
80028148:  or a0,a0,v0
8002814c:  sw a0,0x0(t0)
80028150:  gte_stRGB2 0x14(t0)
80028154:  j 0x800288c8
80028158:  _addiu t0,t0,0x1c
8002815c:  srl a1,t6,0x1
80028160:  addu a0,s0,a1
80028164:  lui a2,0x8009
80028168:  addiu a2,a2,0x1120
8002816c:  srl v0,a0,0x6
80028170:  sll v0,v0,0x2
80028174:  srl v1,t7,0x6
80028178:  sll v1,v1,0x7
8002817c:  addu v0,v0,v1
80028180:  addu v0,v0,a2
80028184:  lw v1,0x80(v0)
80028188:  andi a0,a0,0x3f
8002818c:  sll a0,a0,0x1
80028190:  andi v0,t7,0x3f
80028194:  sll v0,v0,0x7
80028198:  addu a0,a0,v0
8002819c:  addu v1,v1,a0
800281a0:  lhu v0,0x0(v1)
800281a4:  srl a0,t2,0x1
800281a8:  addu v1,t5,a0
800281ac:  sh t3,0x38(sp)
800281b0:  sh v1,0x3c(sp)
800281b4:  move t1,v0
800281b8:  andi v0,v0,0x7ff
800281bc:  sll v0,v0,0x3
800281c0:  addu v0,t4,v0
800281c4:  sh v0,0x3a(sp)
800281c8:  gte_ldVXY0 0x28(sp)
800281cc:  gte_ldVZ0 0x2c(sp)
800281d0:  nRTPS
800281dc:  gte_stSXY2 0x8(t0)
800281e0:  srl v0,s2,0xb
800281e4:  sll v0,v0,0x7
800281e8:  gte_ldIR1 v0
800281ec:  nCDP
800281f8:  gte_ldVXY0 0x38(sp)
800281fc:  gte_ldVZ0 0x3c(sp)
80028200:  nRTPS
8002820c:  gte_stSXY2 0x10(t0)
80028210:  srl v0,t1,0xb
80028214:  sll v0,v0,0x7
80028218:  gte_ldIR1 v0
8002821c:  nCDP
80028228:  gte_ldVXY0 0x30(sp)
8002822c:  gte_ldVZ0 0x34(sp)
80028230:  nRTPS
8002823c:  gte_stSXY2 0x18(t0)
80028240:  srl v0,s4,0xb
80028244:  sll v0,v0,0x7
80028248:  gte_ldIR1 v0
8002824c:  nCDP
80028258:  addu a1,t7,a1
8002825c:  srl v0,s0,0x6
80028260:  sll v0,v0,0x2
80028264:  srl v1,a1,0x6
80028268:  sll v1,v1,0x7
8002826c:  addu v0,v0,v1
80028270:  addu v0,v0,a2
80028274:  lw v1,0x80(v0)
80028278:  andi v0,s0,0x3f
8002827c:  sll v0,v0,0x1
80028280:  andi a1,a1,0x3f
80028284:  sll a1,a1,0x7
80028288:  addu v0,v0,a1
8002828c:  addu v1,v1,v0
80028290:  lhu v0,0x0(v1)
80028294:  addu a0,t3,a0
80028298:  sh a0,0x40(sp)
8002829c:  sh t5,0x44(sp)
800282a0:  move a1,v0
800282a4:  andi v0,v0,0x7ff
800282a8:  sll v0,v0,0x3
800282ac:  addu v0,t4,v0
800282b0:  sh v0,0x42(sp)
800282b4:  NCLIP
800282b8:  gte_stMAC0 v0
800282bc:  nop
800282c0:  bgez v0,0x80028318
800282c4:  _nop
800282c8:  strgb3_g3 t0
800282d4:  nAVSZ3
800282e0:  gte_stOTZ v1
800282e4:  lui v0,0x1f80
800282e8:  lw v0,0x0(v0)
800282ec:  srl v1,v1,0x1
800282f0:  sll v1,v1,0x2
800282f4:  addu v1,v1,v0
800282f8:  lw a0,0x0(v1)
800282fc:  sll v0,t0,0x8
80028300:  srl v0,v0,0x8
80028304:  sw v0,0x0(v1)
80028308:  lui v0,0x600
8002830c:  or a0,a0,v0
80028310:  sw a0,0x0(t0)
80028314:  addiu t0,t0,0x1c
80028318:  gte_ldVXY0 0x40(sp)
8002831c:  gte_ldVZ0 0x44(sp)
80028320:  nRTPS
8002832c:  gte_stOTZ v0
80028330:  srl v0,a1,0xb
80028334:  sll v0,v0,0x7
80028338:  gte_ldIR1 v0
8002833c:  nCDP
80028348:  gte_stRGB0 0x3c(t0)
8002834c:  gte_stSXY0 0x40(t0)
80028350:  gte_stRGB2 0x44(t0)
80028354:  gte_stSXY2 0x48(t0)
80028358:  nNCLIP
80028364:  gte_stMAC0 v0
80028368:  nop
8002836c:  blez v0,0x800283cc
80028370:  _nop
80028374:  stsxy3_g3 t0
80028380:  strgb3_g3 t0
8002838c:  nAVSZ3
80028398:  gte_stOTZ v1
8002839c:  lui v0,0x1f80
800283a0:  lw v0,0x0(v0)
800283a4:  srl v1,v1,0x1
800283a8:  sll v1,v1,0x2
800283ac:  addu v1,v1,v0
800283b0:  lw a0,0x0(v1)
800283b4:  sll v0,t0,0x8
800283b8:  srl v0,v0,0x8
800283bc:  sw v0,0x0(v1)
800283c0:  lui v0,0x600
800283c4:  or a0,a0,v0
800283c8:  sw a0,0x0(t0)
800283cc:  gte_ldVXY0 0x20(sp)
800283d0:  gte_ldVZ0 0x24(sp)
800283d4:  nRTPS
800283e0:  gte_stOTZ v0
800283e4:  srl v0,s5,0xb
800283e8:  sll v0,v0,0x7
800283ec:  gte_ldIR1 v0
800283f0:  nCDP
800283fc:  addiu t0,t0,0x1c
80028400:  nop
80028404:  NCLIP
80028408:  gte_stMAC0 v0
8002840c:  nop
80028410:  bgez v0,0x80028470
80028414:  _nop
80028418:  stsxy3_g3 t0
80028424:  strgb3_g3 t0
80028430:  nAVSZ3
8002843c:  gte_stOTZ v1
80028440:  lui v0,0x1f80
80028444:  lw v0,0x0(v0)
80028448:  srl v1,v1,0x1
8002844c:  sll v1,v1,0x2
80028450:  addu v1,v1,v0
80028454:  lw a0,0x0(v1)
80028458:  sll v0,t0,0x8
8002845c:  srl v0,v0,0x8
80028460:  sw v0,0x0(v1)
80028464:  lui v0,0x600
80028468:  or a0,a0,v0
8002846c:  sw a0,0x0(t0)
80028470:  gte_ldVXY0 0x18(sp)
80028474:  gte_ldVZ0 0x1c(sp)
80028478:  nRTPS
80028484:  gte_stSXY2 0x34(t0)
80028488:  gte_ldSXY0 0x24(t0)
8002848c:  gte_ldSXY1 0x2c(t0)
80028490:  addiu t0,t0,0x1c
80028494:  nop
80028498:  NCLIP
8002849c:  gte_stMAC0 v0
800284a0:  nop
800284a4:  blez v0,0x800288c8
800284a8:  _nop
800284ac:  nAVSZ3
800284b8:  gte_stOTZ a1
800284bc:  srl v0,s3,0xb
800284c0:  sll v0,v0,0x7
800284c4:  gte_ldIR1 v0
800284c8:  nCDP
800284d4:  lui v0,0x1f80
800284d8:  lw v0,0x0(v0)
800284dc:  srl v1,a1,0x1
800284e0:  sll v1,v1,0x2
800284e4:  addu v1,v1,v0
800284e8:  lw a0,0x0(v1)
800284ec:  sll v0,t0,0x8
800284f0:  srl v0,v0,0x8
800284f4:  sw v0,0x0(v1)
800284f8:  lui v0,0x600
800284fc:  or a0,a0,v0
80028500:  sw a0,0x0(t0)
80028504:  gte_stRGB2 0x14(t0)
80028508:  j 0x800288c8
8002850c:  _addiu t0,t0,0x1c
80028510:  srl a2,t6,0x1
80028514:  addu a1,t7,t6
80028518:  addu a0,s0,a2
8002851c:  lui a3,0x8009
80028520:  addiu a3,a3,0x1120
80028524:  srl v0,a0,0x6
80028528:  sll v0,v0,0x2
8002852c:  srl v1,a1,0x6
80028530:  sll v1,v1,0x7
80028534:  addu v0,v0,v1
80028538:  addu v0,v0,a3
8002853c:  lw v0,0x80(v0)
80028540:  andi a0,a0,0x3f
80028544:  sll a0,a0,0x1
80028548:  andi a1,a1,0x3f
8002854c:  sll a1,a1,0x7
80028550:  addu a0,a0,a1
80028554:  addu v0,v0,a0
80028558:  lhu v0,0x0(v0)
8002855c:  addu v1,t3,t2
80028560:  srl a0,t2,0x1
80028564:  sh v1,0x38(sp)
80028568:  addu v1,t5,a0
8002856c:  sh v1,0x3c(sp)
80028570:  move t1,v0
80028574:  andi v0,v0,0x7ff
80028578:  sll v0,v0,0x3
8002857c:  addu v0,t4,v0
80028580:  sh v0,0x3a(sp)
80028584:  gte_ldVXY0 0x30(sp)
80028588:  gte_ldVZ0 0x34(sp)
8002858c:  nRTPS
80028598:  gte_stSXY2 0x8(t0)
8002859c:  srl v0,s4,0xb
800285a0:  sll v0,v0,0x7
800285a4:  gte_ldIR1 v0
800285a8:  nCDP
800285b4:  gte_ldVXY0 0x38(sp)
800285b8:  gte_ldVZ0 0x3c(sp)
800285bc:  nRTPS
800285c8:  gte_stSXY2 0x10(t0)
800285cc:  srl v0,t1,0xb
800285d0:  sll v0,v0,0x7
800285d4:  gte_ldIR1 v0
800285d8:  nCDP
800285e4:  gte_ldVXY0 0x28(sp)
800285e8:  gte_ldVZ0 0x2c(sp)
800285ec:  nRTPS
800285f8:  gte_stSXY2 0x18(t0)
800285fc:  srl v0,s2,0xb
80028600:  sll v0,v0,0x7
80028604:  gte_ldIR1 v0
80028608:  nCDP
80028614:  addu a2,t7,a2
80028618:  srl v0,s0,0x6
8002861c:  sll v0,v0,0x2
80028620:  srl v1,a2,0x6
80028624:  sll v1,v1,0x7
80028628:  addu v0,v0,v1
8002862c:  addu v0,v0,a3
80028630:  lw v1,0x80(v0)
80028634:  andi v0,s0,0x3f
80028638:  sll v0,v0,0x1
8002863c:  andi a2,a2,0x3f
80028640:  sll a2,a2,0x7
80028644:  addu v0,v0,a2
80028648:  addu v1,v1,v0
8002864c:  lhu v0,0x0(v1)
80028650:  addu a0,t3,a0
80028654:  sh a0,0x40(sp)
80028658:  sh t5,0x44(sp)
8002865c:  move a1,v0
80028660:  andi v0,v0,0x7ff
80028664:  sll v0,v0,0x3
80028668:  addu v0,t4,v0
8002866c:  sh v0,0x42(sp)
80028670:  NCLIP
80028674:  gte_stMAC0 v0
80028678:  nop
8002867c:  blez v0,0x800286d4
80028680:  _nop
80028684:  strgb3_g3 t0
80028690:  nAVSZ3
8002869c:  gte_stOTZ v1
800286a0:  lui v0,0x1f80
800286a4:  lw v0,0x0(v0)
800286a8:  srl v1,v1,0x1
800286ac:  sll v1,v1,0x2
800286b0:  addu v1,v1,v0
800286b4:  lw a0,0x0(v1)
800286b8:  sll v0,t0,0x8
800286bc:  srl v0,v0,0x8
800286c0:  sw v0,0x0(v1)
800286c4:  lui v0,0x600
800286c8:  or a0,a0,v0
800286cc:  sw a0,0x0(t0)
800286d0:  addiu t0,t0,0x1c
800286d4:  gte_ldVXY0 0x40(sp)
800286d8:  gte_ldVZ0 0x44(sp)
800286dc:  nRTPS
800286e8:  gte_stOTZ v0
800286ec:  srl v0,a1,0xb
800286f0:  sll v0,v0,0x7
800286f4:  gte_ldIR1 v0
800286f8:  nCDP
80028704:  gte_stRGB0 0x3c(t0)
80028708:  gte_stSXY0 0x40(t0)
8002870c:  gte_stRGB2 0x44(t0)
80028710:  gte_stSXY2 0x48(t0)
80028714:  nNCLIP
80028720:  gte_stMAC0 v0
80028724:  nop
80028728:  bgez v0,0x80028788
8002872c:  _nop
80028730:  stsxy3_g3 t0
8002873c:  strgb3_g3 t0
80028748:  nAVSZ3
80028754:  gte_stOTZ v1
80028758:  lui v0,0x1f80
8002875c:  lw v0,0x0(v0)
80028760:  srl v1,v1,0x1
80028764:  sll v1,v1,0x2
80028768:  addu v1,v1,v0
8002876c:  lw a0,0x0(v1)
80028770:  sll v0,t0,0x8
80028774:  srl v0,v0,0x8
80028778:  sw v0,0x0(v1)
8002877c:  lui v0,0x600
80028780:  or a0,a0,v0
80028784:  sw a0,0x0(t0)
80028788:  gte_ldVXY0 0x18(sp)
8002878c:  gte_ldVZ0 0x1c(sp)
80028790:  nRTPS
8002879c:  gte_stOTZ v0
800287a0:  srl v0,s3,0xb
800287a4:  sll v0,v0,0x7
800287a8:  gte_ldIR1 v0
800287ac:  nCDP
800287b8:  addiu t0,t0,0x1c
800287bc:  nop
800287c0:  NCLIP
800287c4:  gte_stMAC0 v0
800287c8:  nop
800287cc:  blez v0,0x8002882c
800287d0:  _nop
800287d4:  stsxy3_g3 t0
800287e0:  strgb3_g3 t0
800287ec:  nAVSZ3
800287f8:  gte_stOTZ v1
800287fc:  lui v0,0x1f80
80028800:  lw v0,0x0(v0)
80028804:  srl v1,v1,0x1
80028808:  sll v1,v1,0x2
8002880c:  addu v1,v1,v0
80028810:  lw a0,0x0(v1)
80028814:  sll v0,t0,0x8
80028818:  srl v0,v0,0x8
8002881c:  sw v0,0x0(v1)
80028820:  lui v0,0x600
80028824:  or a0,a0,v0
80028828:  sw a0,0x0(t0)
8002882c:  gte_ldVXY0 0x20(sp)
80028830:  gte_ldVZ0 0x24(sp)
80028834:  nRTPS
80028840:  gte_stSXY2 0x34(t0)
80028844:  gte_ldSXY0 0x24(t0)
80028848:  gte_ldSXY1 0x2c(t0)
8002884c:  addiu t0,t0,0x1c
80028850:  nop
80028854:  NCLIP
80028858:  gte_stMAC0 v0
8002885c:  nop
80028860:  bgez v0,0x800288c8
80028864:  _nop
80028868:  nAVSZ3
80028874:  gte_stOTZ a1
80028878:  srl v0,s5,0xb
8002887c:  sll v0,v0,0x7
80028880:  gte_ldIR1 v0
80028884:  nCDP
80028890:  lui v0,0x1f80
80028894:  lw v0,0x0(v0)
80028898:  srl v1,a1,0x1
8002889c:  sll v1,v1,0x2
800288a0:  addu v1,v1,v0
800288a4:  lw a0,0x0(v1)
800288a8:  sll v0,t0,0x8
800288ac:  srl v0,v0,0x8
800288b0:  sw v0,0x0(v1)
800288b4:  lui v0,0x600
800288b8:  or a0,a0,v0
800288bc:  sw a0,0x0(t0)
800288c0:  gte_stRGB2 0x14(t0)
800288c4:  addiu t0,t0,0x1c
800288c8:  lui at,0x1f80
800288cc:  sw t0,0x14(at)
800288d0:  lw ra,0x68(sp)
800288d4:  lw s7,0x64(sp)
800288d8:  lw s6,0x60(sp)
800288dc:  lw s5,0x5c(sp)
800288e0:  lw s4,0x58(sp)
800288e4:  lw s3,0x54(sp)
800288e8:  lw s2,0x50(sp)
800288ec:  lw s1,0x4c(sp)
800288f0:  lw s0,0x48(sp)
800288f4:  jr ra
800288f8:  _addiu sp,sp,0x70
