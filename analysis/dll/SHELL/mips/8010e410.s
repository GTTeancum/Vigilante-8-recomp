# addr: 0x8010e410  name: FUN_8010e410
8010e410:  addiu sp,sp,-0x18
8010e414:  move a1,a0
8010e418:  beq a1,zero,0x8010e434
8010e41c:  _sw ra,0x10(sp)
8010e420:  li v0,0x1
8010e424:  beq a1,v0,0x8010e494
8010e428:  _lui v0,0x8000
8010e42c:  slti a0,zero,0x3939
8010e430:  nop
8010e434:  lui v1,0x8011
8010e438:  lw v1,0x24e8(v1)
8010e43c:  lui v0,0x8000
8010e440:  sw v0,0x0(v1)
8010e444:  lui v0,0x8011
8010e448:  lw v0,0x24bc(v0)
8010e44c:  lui a0,0x8011
8010e450:  addiu a0,a0,0x23a4
8010e454:  sw zero,0x0(v0)
8010e458:  lui v0,0x8011
8010e45c:  lw v0,0x24c8(v0)
8010e460:  li a1,0x20
8010e464:  sw zero,0x0(v0)
8010e468:  lui v1,0x8011
8010e46c:  lw v1,0x24e8(v1)
8010e470:  lui v0,0x6000
8010e474:  sltiu a0,zero,0x3940
8010e478:  sw v0,0x0(v1)
8010e47c:  lui a0,0x8011
8010e480:  addiu a0,a0,0x2428
8010e484:  sltiu a0,zero,0x3940
8010e488:  li a1,0x20
8010e48c:  slti a0,zero,0x393c
8010e490:  nop
8010e494:  lui v1,0x8011
8010e498:  lw v1,0x24e8(v1)
8010e49c:  nop
8010e4a0:  sw v0,0x0(v1)
8010e4a4:  lui v0,0x8011
8010e4a8:  lw v0,0x24bc(v0)
8010e4ac:  nop
8010e4b0:  sw zero,0x0(v0)
8010e4b4:  lui v0,0x8011
8010e4b8:  lw v0,0x24c8(v0)
8010e4bc:  nop
8010e4c0:  sw zero,0x0(v0)
8010e4c4:  lui v0,0x8011
8010e4c8:  lw v0,0x24c8(v0)
8010e4cc:  lui v1,0x8011
8010e4d0:  lw v1,0x24e8(v1)
8010e4d4:  lw v0,0x0(v0)
8010e4d8:  lui v0,0x6000
8010e4dc:  slti a0,zero,0x393c
8010e4e0:  sw v0,0x0(v1)
8010e4e4:  lui a0,0x8010
8010e4e8:  jal 0x80052604
8010e4ec:  _addiu a0,a0,0x1094
8010e4f0:  lw ra,0x10(sp)
8010e4f4:  addiu sp,sp,0x18
8010e4f8:  jr ra
8010e4fc:  _nop
