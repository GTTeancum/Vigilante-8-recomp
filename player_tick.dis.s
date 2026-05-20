# addr: 0x8002e2bc  name: LAB_8002e2bc (PLAYER VEHICLE TICK)
# Recovered by static-analysis on 2026-05-20.
# Assigned to obj+0x64 (tick callback) by FUN_8002e630 line 34.
# Size: 0x374 bytes (221 instructions).

0x8002e2bc: addiu sp,sp,-104
0x8002e2c0: sw s0,80(sp)
0x8002e2c4: addu s0,a0,zero
0x8002e2c8: sw s4,96(sp)
0x8002e2cc: addu s4,a2,zero
0x8002e2d0: addiu v0,zero,2
0x8002e2d4: sw ra,100(sp)
0x8002e2d8: sw s3,92(sp)
0x8002e2dc: sw s2,88(sp)
0x8002e2e0: beq a1,v0,0x8002e5b4
0x8002e2e4: sw s1,84(sp)
0x8002e2e8: sltiu v0,a1,3
0x8002e2ec: beq v0,zero,0x8002e304
0x8002e2f0: addiu v0,zero,3
0x8002e2f4: beq a1,zero,0x8002e31c
0x8002e2f8: addu v0,zero,zero
0x8002e2fc: j 0x8002e5e4
0x8002e300: nop
0x8002e304: beq a1,v0,0x8002e5c4
0x8002e308: addiu v0,zero,4
0x8002e30c: beq a1,v0,0x8002e5d8
0x8002e310: addu v0,zero,zero
0x8002e314: j 0x8002e5e4
0x8002e318: nop
0x8002e31c: lh v0,6(s0)
0x8002e320: nop
0x8002e324: bgez v0,0x8002e454
0x8002e328: addiu s2,s0,164
0x8002e32c: lw a0,36(s0)
0x8002e330: lw a1,44(s0)
0x8002e334: jal 0x800255f4
0x8002e338: nop
0x8002e33c: lh v1,6(s0)
0x8002e340: addu s1,v0,zero
0x8002e344: nor v1,zero,v1
0x8002e348: sll a0,v1,1
0x8002e34c: addu a0,a0,v1
0x8002e350: lh v1,22(s1)
0x8002e354: lui v0,0x8006
0x8002e358: addiu v0,v0,23592
0x8002e35c: sll a0,a0,3
0x8002e360: addu s3,a0,v0
0x8002e364: addiu v0,zero,7
0x8002e368: bne v1,v0,0x8002e394
0x8002e36c: nop
0x8002e370: lw v0,128(s0)
0x8002e374: lw v1,132(s0)
0x8002e378: lw a0,136(s0)
0x8002e37c: subu v0,zero,v0
0x8002e380: subu v1,zero,v1
0x8002e384: subu a0,zero,a0
0x8002e388: sw v0,128(s0)
0x8002e38c: sw v1,132(s0)
0x8002e390: sw a0,136(s0)
0x8002e394: lw v0,0(s0)
0x8002e398: lui v1,0x20
0x8002e39c: and v0,v0,v1
0x8002e3a0: bne v0,zero,0x8002e3cc
0x8002e3a4: nop
0x8002e3a8: lbu v0,211(s0)
0x8002e3ac: nop
0x8002e3b0: beq v0,zero,0x8002e428
0x8002e3b4: addu a0,s0,zero
0x8002e3b8: jal 0x800441c8
0x8002e3bc: addu a0,v0,zero
0x8002e3c0: sb zero,210(s0)
0x8002e3c4: j 0x8002e424
0x8002e3c8: sb zero,211(s0)
0x8002e3cc: lh v1,24(s1)
0x8002e3d0: lbu v0,210(s0)
0x8002e3d4: nop
0x8002e3d8: beq v1,v0,0x8002e428
0x8002e3dc: addu a0,s0,zero
0x8002e3e0: lbu a0,211(s0)
0x8002e3e4: jal 0x800441c8
0x8002e3e8: nop
0x8002e3ec: lbu v0,24(s1)
0x8002e3f0: nop
0x8002e3f4: sb v0,210(s0)
0x8002e3f8: andi v0,v0,0xff
0x8002e3fc: beq v0,zero,0x8002e41c
0x8002e400: addu a0,s0,zero
0x8002e404: lw v0,1840(gp)
0x8002e408: addiu a1,zero,11
0x8002e40c: jalr v0
0x8002e410: addu a2,s1,zero
0x8002e414: j 0x8002e424
0x8002e418: sb v0,211(s0)
0x8002e41c: addu v0,zero,zero
0x8002e420: sb v0,211(s0)
0x8002e424: addu a0,s0,zero
0x8002e428: jal 0x8002efe0
0x8002e42c: addu a1,s3,zero
0x8002e430: addu a0,s0,zero
0x8002e434: jal 0x8002d494
0x8002e438: addu a1,s3,zero
0x8002e43c: beq s4,zero,0x8002e4cc
0x8002e440: nop
0x8002e444: jal 0x8002d054
0x8002e448: addu a0,s0,zero
0x8002e44c: j 0x8002e4cc
0x8002e450: nop
0x8002e454: lhu v0,176(s0)
0x8002e458: nop
0x8002e45c: beq v0,zero,0x8002e4c4
0x8002e460: addiu v0,v0,-1
0x8002e464: sh v0,176(s0)
0x8002e468: andi v0,v0,0xffff
0x8002e46c: bne v0,zero,0x8002e4a0
0x8002e470: addiu a2,zero,31
0x8002e474: lb a0,5(s0)
0x8002e478: lw a1,1528(gp)
0x8002e47c: jal 0x800447e8
0x8002e480: addiu a3,s0,36
0x8002e484: lw v0,0(s0)
0x8002e488: lui v1,0xf7ff
0x8002e48c: ori v1,v1,0xffff
0x8002e490: sb zero,5(s0)
0x8002e494: and v0,v0,v1
0x8002e498: j 0x8002e4cc
0x8002e49c: sw v0,0(s0)
0x8002e4a0: beq s4,zero,0x8002e4cc
0x8002e4a4: nop
0x8002e4a8: jal 0x800446dc
0x8002e4ac: addiu a0,s0,36
0x8002e4b0: lb a0,5(s0)
0x8002e4b4: jal 0x80044574
0x8002e4b8: addu a1,v0,zero
0x8002e4bc: j 0x8002e4cc
0x8002e4c0: nop
0x8002e4c4: jal 0x80023940
0x8002e4c8: addu a0,s0,zero
0x8002e4cc: lbu v0,18(s2)
0x8002e4d0: nop
0x8002e4d4: beq v0,zero,0x8002e568
0x8002e4d8: addiu v0,v0,-1
0x8002e4dc: sb v0,18(s2)
0x8002e4e0: andi v0,v0,0xff
0x8002e4e4: bne v0,zero,0x8002e568
0x8002e4e8: nop
0x8002e4ec: lbu v0,21(s2)
0x8002e4f0: nop
0x8002e4f4: beq v0,zero,0x8002e568
0x8002e4f8: nop
0x8002e4fc: lh v0,6(s0)
0x8002e500: nop
0x8002e504: bgez v0,0x8002e54c
0x8002e508: addiu a0,sp,16
0x8002e50c: lbu a2,21(s2)
0x8002e510: lui a1,0x8006
0x8002e514: jal 0x80053004
0x8002e518: addiu a1,a1,22328
0x8002e51c: lw v0,16(gp)
0x8002e520: nop
0x8002e524: beq v0,zero,0x8002e538
0x8002e528: nop
0x8002e52c: lh v0,6(s0)
0x8002e530: j 0x8002e53c
0x8002e534: subu a0,zero,v0
0x8002e538: addu a0,zero,zero
0x8002e53c: jal 0x800129e8
0x8002e540: addiu a1,sp,16
0x8002e544: jal 0x8002d44c
0x8002e548: addu a0,s0,zero
0x8002e54c: lbu v0,22(s2)
0x8002e550: lbu a0,21(s2)
0x8002e554: addiu v1,zero,30
0x8002e558: sb v1,18(s2)
0x8002e55c: sb zero,21(s2)
0x8002e560: addu v0,v0,a0
0x8002e564: sb v0,22(s2)
0x8002e568: beq s4,zero,0x8002e594
0x8002e56c: lui v1,0xfffe
0x8002e570: lw v0,0(s0)
0x8002e574: ori v1,v1,0x7fff
0x8002e578: and a0,v0,v1
0x8002e57c: lui v1,0x1
0x8002e580: and v0,v0,v1
0x8002e584: beq v0,zero,0x8002e590
0x8002e588: nop
0x8002e58c: ori a0,a0,0x8000
0x8002e590: sw a0,0(s0)
0x8002e594: lh v0,6(s0)
0x8002e598: nop
0x8002e59c: beq v0,zero,0x8002e5b4
0x8002e5a0: addu a0,s0,zero
0x8002e5a4: jal 0x80020890
0x8002e5a8: addu a1,zero,zero
0x8002e5ac: j 0x8002e5e4
0x8002e5b0: addu v0,zero,zero
0x8002e5b4: jal 0x8002f998
0x8002e5b8: addu a0,s0,zero
0x8002e5bc: j 0x8002e5e4
0x8002e5c0: addu v0,zero,zero
0x8002e5c4: addu a0,s0,zero
0x8002e5c8: jal 0x8002d82c
0x8002e5cc: addu a1,s4,zero
0x8002e5d0: j 0x8002e5e4
0x8002e5d4: nop
0x8002e5d8: jal 0x80042f5c
0x8002e5dc: addiu a0,s0,192
0x8002e5e0: addu v0,zero,zero
0x8002e5e4: lw ra,100(sp)
0x8002e5e8: lw s4,96(sp)
0x8002e5ec: lw s3,92(sp)
0x8002e5f0: lw s2,88(sp)
0x8002e5f4: lw s1,84(sp)
0x8002e5f8: lw s0,80(sp)
0x8002e5fc: jr ra
0x8002e600: addiu sp,sp,104
0x8002e604: addu v1,zero,zero
0x8002e608: sh a1,14(a0)
0x8002e60c: sh a1,12(a0)
0x8002e610: lw v0,236(a0)
0x8002e614: addiu v1,v1,1
0x8002e618: sh a1,12(v0)
0x8002e61c: slti v0,v1,3
0x8002e620: bne v0,zero,0x8002e610
0x8002e624: addiu a0,a0,4
0x8002e628: jr ra
0x8002e62c: nop
