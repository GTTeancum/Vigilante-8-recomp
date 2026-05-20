# addr: 0x80100624  name: FUN_80100624
80100624:  addiu sp,sp,-0x20
80100628:  move a3,a0
8010062c:  sw ra,0x18(sp)
80100630:  lw a0,0x5c(a3)
80100634:  lw v1,0x48(a3)
80100638:  lw v0,0x4(a0)
8010063c:  nop
80100640:  addu v1,v1,v0
80100644:  bgez v1,0x80100654
80100648:  _addiu a0,a0,0x4
8010064c:  ori v0,zero,0xffff
80100650:  addu v1,v1,v0
80100654:  sra t0,v1,0x10
80100658:  sh t0,0x10(sp)
8010065c:  lw v1,0x50(a3)
80100660:  lw v0,0x8(a0)
80100664:  nop
80100668:  addu v1,v1,v0
8010066c:  bgez v1,0x80100680
80100670:  _sra t1,v1,0x10
80100674:  ori v0,zero,0xffff
80100678:  addu v1,v1,v0
8010067c:  sra t1,v1,0x10
80100680:  sh t1,0x12(sp)
80100684:  lw v0,0x48(a3)
80100688:  lw v1,0xc(a0)
8010068c:  ori a2,zero,0xffff
80100690:  addu v0,v0,v1
80100694:  addu v0,v0,a2
80100698:  bgez v0,0x801006a4
8010069c:  _nop
801006a0:  addu v0,v0,a2
801006a4:  sra v0,v0,0x10
801006a8:  subu v0,v0,t0
801006ac:  sh v0,0x14(sp)
801006b0:  lw v0,0x50(a3)
801006b4:  lw v1,0x14(a0)
801006b8:  nop
801006bc:  addu v0,v0,v1
801006c0:  addu v0,v0,a2
801006c4:  bgez v0,0x801006d0
801006c8:  _nop
801006cc:  addu v0,v0,a2
801006d0:  sra v0,v0,0x10
801006d4:  subu v0,v0,t1
801006d8:  addiu a0,sp,0x10
801006dc:  jal 0x80024718
801006e0:  _sh v0,0x16(sp)
801006e4:  lw ra,0x18(sp)
801006e8:  nop
801006ec:  jr ra
801006f0:  _addiu sp,sp,0x20
