# addr: 0x801005e0  name: FUN_801005e0
801005e0:  addiu sp,sp,-0x58
801005e4:  sw s4,0x50(sp)
801005e8:  move s4,a0
801005ec:  sw s3,0x4c(sp)
801005f0:  move s3,a1
801005f4:  andi v0,a1,0x7fff
801005f8:  slti v0,v0,0x7000
801005fc:  sw ra,0x54(sp)
80100600:  sw s2,0x48(sp)
80100604:  sw s1,0x44(sp)
80100608:  sw s0,0x40(sp)
8010060c:  beq v0,zero,0x801007d0
80100610:  _sh a1,0x46(s4)
80100614:  sll v0,a1,0x10
80100618:  sra v0,v0,0x10
8010061c:  bgez v0,0x8010062c
80100620:  _move s1,v0
80100624:  subu v0,zero,v0
80100628:  addiu s1,v0,-0x1000
8010062c:  lui v0,0x8006
80100630:  lw v0,0x59fc(v0)
80100634:  nop
80100638:  lw a0,0x80(v0)
8010063c:  nop
80100640:  lw a2,0x0(a0)
80100644:  nop
80100648:  beq a2,zero,0x80100678
8010064c:  _nop
80100650:  lw v0,0xc(a2)
80100654:  nop
80100658:  sltu v0,v0,s1
8010065c:  beq v0,zero,0x80100678
80100660:  _nop
80100664:  move a0,a2
80100668:  lw a2,0x0(a2)
8010066c:  nop
80100670:  bne a2,zero,0x80100650
80100674:  _nop
80100678:  lw a1,0xc(a0)
8010067c:  lw s0,0x8(a0)
80100680:  lw v1,0xc(a2)
80100684:  lh a0,0x42(s0)
80100688:  lw s2,0x8(a2)
8010068c:  subu v0,s1,a1
80100690:  sll v0,v0,0x8
80100694:  subu v1,v1,a1
80100698:  divu v0,v1
8010069c:  mflo s1
801006a0:  bgez a0,0x801006ac
801006a4:  _nop
801006a8:  subu a0,zero,a0
801006ac:  slti a0,a0,0x401
801006b0:  bne a0,zero,0x801006bc
801006b4:  _ori a3,zero,0x8000
801006b8:  ori a3,zero,0x8001
801006bc:  sll v0,s3,0x10
801006c0:  bltz v0,0x801006d0
801006c4:  _nop
801006c8:  slti a0,zero,0x1b5
801006cc:  xori a1,a3,0x1
801006d0:  andi a1,a3,0xffff
801006d4:  jal 0x8001b038
801006d8:  _move a0,s0
801006dc:  addiu a0,s0,0x10
801006e0:  addiu a1,v0,0x4
801006e4:  jal 0x80043408
801006e8:  _addiu a2,sp,0x10
801006ec:  lh v0,0x42(s2)
801006f0:  nop
801006f4:  bgez v0,0x80100700
801006f8:  _nop
801006fc:  subu v0,zero,v0
80100700:  slti v0,v0,0x401
80100704:  bne v0,zero,0x80100710
80100708:  _ori a1,zero,0x8000
8010070c:  ori a1,zero,0x8001
80100710:  sll v0,s3,0x10
80100714:  bltz v0,0x80100724
80100718:  _nop
8010071c:  slti a0,zero,0x1ca
80100720:  xori a1,a1,0x1
80100724:  andi a1,a1,0xffff
80100728:  jal 0x8001b038
8010072c:  _move a0,s2
80100730:  addiu a0,s2,0x10
80100734:  addiu a1,v0,0x4
80100738:  jal 0x80043408
8010073c:  _addiu a2,sp,0x20
80100740:  lw v0,0x20(sp)
80100744:  lw v1,0x10(sp)
80100748:  nop
8010074c:  subu v0,v0,v1
80100750:  mult v0,s1
80100754:  mflo v0
80100758:  bgez v0,0x80100764
8010075c:  _nop
80100760:  addiu v0,v0,0xff
80100764:  sra v0,v0,0x8
80100768:  addu v0,v1,v0
8010076c:  sw v0,0x48(s4)
80100770:  lw v0,0x24(sp)
80100774:  lw v1,0x14(sp)
80100778:  nop
8010077c:  subu v0,v0,v1
80100780:  mult v0,s1
80100784:  mflo v0
80100788:  bgez v0,0x80100794
8010078c:  _nop
80100790:  addiu v0,v0,0xff
80100794:  sra v0,v0,0x8
80100798:  addu v0,v1,v0
8010079c:  sw v0,0x4c(s4)
801007a0:  lw v0,0x28(sp)
801007a4:  lw v1,0x18(sp)
801007a8:  nop
801007ac:  subu v0,v0,v1
801007b0:  mult v0,s1
801007b4:  mflo v0
801007b8:  bgez v0,0x801007c4
801007bc:  _nop
801007c0:  addiu v0,v0,0xff
801007c4:  sra v0,v0,0x8
801007c8:  slti a0,zero,0x252
801007cc:  addu v0,v1,v0
801007d0:  andi v1,s3,0xffff
801007d4:  ori v0,zero,0xefff
801007d8:  sltu v0,v0,v1
801007dc:  beq v0,zero,0x801007f8
801007e0:  _lui v0,0x8006
801007e4:  lw v0,0x59fc(v0)
801007e8:  nop
801007ec:  lw v0,0x80(v0)
801007f0:  slti a0,zero,0x201
801007f4:  nop
801007f8:  lw v0,0x59fc(v0)
801007fc:  nop
80100800:  lw v0,0x88(v0)
80100804:  nop
80100808:  lw s1,0x8(v0)
8010080c:  nop
80100810:  move a0,s1
80100814:  jal 0x8001b038
80100818:  _ori a1,zero,0x8000
8010081c:  move a0,s1
80100820:  ori a1,zero,0x8001
80100824:  jal 0x8001b038
80100828:  _move s0,v0
8010082c:  lw t0,0x4(v0)
80100830:  lw a3,0x4(s0)
80100834:  nop
80100838:  addu v1,a3,t0
8010083c:  srl a0,v1,0x1f
80100840:  addu v1,v1,a0
80100844:  sra v1,v1,0x1
80100848:  sw v1,0x30(sp)
8010084c:  lw v1,0x8(s0)
80100850:  lw a0,0x8(v0)
80100854:  addiu a1,sp,0x30
80100858:  addu v1,v1,a0
8010085c:  srl a0,v1,0x1f
80100860:  addu v1,v1,a0
80100864:  sra v1,v1,0x1
80100868:  sw v1,0x4(a1)
8010086c:  lw v1,0xc(s0)
80100870:  lw v0,0xc(v0)
80100874:  move a2,a1
80100878:  addiu a0,s1,0x10
8010087c:  subu s0,t0,a3
80100880:  addu v1,v1,v0
80100884:  srl v0,v1,0x1f
80100888:  addu v1,v1,v0
8010088c:  sra v1,v1,0x1
80100890:  jal 0x80043408
80100894:  _sw v1,0x8(a1)
80100898:  sll v0,s3,0x10
8010089c:  sra v0,v0,0x10
801008a0:  subu v0,zero,v0
801008a4:  srl v1,v0,0x1f
801008a8:  addu v0,v0,v1
801008ac:  sra a0,v0,0x1
801008b0:  andi v1,s3,0xffff
801008b4:  ori v0,zero,0xefff
801008b8:  sltu v0,v0,v1
801008bc:  bne v0,zero,0x801008c8
801008c0:  _lui v0,0x8006
801008c4:  addiu a0,a0,0x800
801008c8:  addiu a1,v0,0x7b4
801008cc:  andi v0,a0,0xfff
801008d0:  sll v0,v0,0x2
801008d4:  addu v0,v0,a1
801008d8:  sh a0,0x42(s4)
801008dc:  lh v0,0x2(v0)
801008e0:  nop
801008e4:  mult v0,s0
801008e8:  mflo v1
801008ec:  bgez v1,0x801008f8
801008f0:  _nop
801008f4:  addiu v1,v1,0x1fff
801008f8:  lw v0,0x30(sp)
801008fc:  sra v1,v1,0xd
80100900:  addu v0,v0,v1
80100904:  sw v0,0x48(s4)
80100908:  lhu v0,0x42(s4)
8010090c:  lw v1,0x34(sp)
80100910:  andi v0,v0,0xfff
80100914:  sll v0,v0,0x2
80100918:  addu v0,v0,a1
8010091c:  sw v1,0x4c(s4)
80100920:  lh v0,0x0(v0)
80100924:  nop
80100928:  mult v0,s0
8010092c:  mflo v1
80100930:  bgez v1,0x8010093c
80100934:  _nop
80100938:  addiu v1,v1,0x1fff
8010093c:  lw v0,0x38(sp)
80100940:  sra v1,v1,0xd
80100944:  subu v0,v0,v1
80100948:  sw v0,0x50(s4)
8010094c:  jal 0x8001d708
80100950:  _move a0,s4
80100954:  lw ra,0x54(sp)
80100958:  lw s4,0x50(sp)
8010095c:  lw s3,0x4c(sp)
80100960:  lw s2,0x48(sp)
80100964:  lw s1,0x44(sp)
80100968:  lw s0,0x40(sp)
8010096c:  jr ra
80100970:  _addiu sp,sp,0x58
