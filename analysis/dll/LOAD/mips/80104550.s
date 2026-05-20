# addr: 0x80104550  name: FUN_80104550
80104550:  addiu sp,sp,-0xc0
80104554:  sw ra,0xbc(sp)
80104558:  sw s8,0xb8(sp)
8010455c:  sw s7,0xb4(sp)
80104560:  sw s6,0xb0(sp)
80104564:  sw s5,0xac(sp)
80104568:  sw s4,0xa8(sp)
8010456c:  sw s3,0xa4(sp)
80104570:  sw s2,0xa0(sp)
80104574:  sw s1,0x9c(sp)
80104578:  sw s0,0x98(sp)
8010457c:  sw a0,0xc0(sp)
80104580:  lw t2,0xc0(sp)
80104584:  nop
80104588:  lw v0,0x0(t2)
8010458c:  nop
80104590:  lw v0,0x0(v0)
80104594:  nop
80104598:  sw v0,0x40(sp)
8010459c:  lw v0,0x0(t2)
801045a0:  nop
801045a4:  lw v0,0x4(v0)
801045a8:  nop
801045ac:  sw v0,0x44(sp)
801045b0:  lw v0,0x0(t2)
801045b4:  nop
801045b8:  lw v0,0x8(v0)
801045bc:  nop
801045c0:  sw v0,0x48(sp)
801045c4:  lw v0,0x0(t2)
801045c8:  lw v1,0x10(t2)
801045cc:  lw v0,0x0(v0)
801045d0:  sw zero,0x50(sp)
801045d4:  addu v0,v0,v1
801045d8:  sw v0,0x4c(sp)
801045dc:  lw v0,0x0(t2)
801045e0:  lw v1,0x14(t2)
801045e4:  lw v0,0x8(v0)
801045e8:  nop
801045ec:  addu v0,v0,v1
801045f0:  sw v0,0x54(sp)
801045f4:  lw v0,0x4(t2)
801045f8:  lw v1,0x18(t2)
801045fc:  lw v0,0x0(v0)
80104600:  sw zero,0x5c(sp)
80104604:  addu v0,v0,v1
80104608:  sw v0,0x58(sp)
8010460c:  lw v0,0x4(t2)
80104610:  lw v1,0x1c(t2)
80104614:  lw v0,0x8(v0)
80104618:  nop
8010461c:  addu v0,v0,v1
80104620:  sw v0,0x60(sp)
80104624:  lw v0,0x4(t2)
80104628:  nop
8010462c:  lw v0,0x0(v0)
80104630:  nop
80104634:  sw v0,0x64(sp)
80104638:  lw v0,0x4(t2)
8010463c:  nop
80104640:  lw v0,0x4(v0)
80104644:  nop
80104648:  sw v0,0x68(sp)
8010464c:  lw v0,0x4(t2)
80104650:  nop
80104654:  lw v0,0x8(v0)
80104658:  addiu a0,sp,0x10
8010465c:  addiu a1,sp,0x70
80104660:  addiu v1,sp,0x40
80104664:  sw v0,0x6c(sp)
80104668:  lw t3,0x0(v1)
8010466c:  lw t4,0x4(v1)
80104670:  lw t5,0x8(v1)
80104674:  lw t2,0xc(v1)
80104678:  sw t3,0x0(a0)
8010467c:  sw t4,0x4(a0)
80104680:  sw t5,0x8(a0)
80104684:  sw t2,0xc(a0)
80104688:  addiu v1,v1,0x10
8010468c:  bne v1,a1,0x80104668
80104690:  _addiu a0,a0,0x10
80104694:  addiu t3,sp,0x60
80104698:  addiu t4,sp,0x40
8010469c:  sw zero,0x80(sp)
801046a0:  sw t3,0x88(sp)
801046a4:  sw t4,0x8c(sp)
801046a8:  sw zero,0x90(sp)
801046ac:  lw t5,0xc0(sp)
801046b0:  nop
801046b4:  sw t5,0x94(sp)
801046b8:  lw t2,0x80(sp)
801046bc:  lw t3,0xc0(sp)
801046c0:  sll v0,t2,0x2
801046c4:  addu v0,t3,v0
801046c8:  lw s4,0x0(v0)
801046cc:  nop
801046d0:  lw v0,0x18(s4)
801046d4:  nop
801046d8:  beq v0,zero,0x80104928
801046dc:  _nop
801046e0:  lw v0,0xc(s4)
801046e4:  nop
801046e8:  lw v0,0x0(v0)
801046ec:  nop
801046f0:  sw v0,0x84(sp)
801046f4:  lw t4,0x94(sp)
801046f8:  nop
801046fc:  lw v0,0x10(t4)
80104700:  sw zero,0x54(sp)
80104704:  sw v0,0x50(sp)
80104708:  lw v0,0x14(t4)
8010470c:  sw zero,0x5c(sp)
80104710:  sw v0,0x58(sp)
80104714:  lw t5,0x50(sp)
80104718:  lw t2,0x54(sp)
8010471c:  lw t3,0x58(sp)
80104720:  lw t4,0x5c(sp)
80104724:  sw t5,0x40(sp)
80104728:  sw t2,0x44(sp)
8010472c:  sw t3,0x48(sp)
80104730:  sw t4,0x4c(sp)
80104734:  lh a0,0x14(s4)
80104738:  lw t5,0x84(sp)
8010473c:  lhu v1,0x16(s4)
80104740:  sll v0,a0,0x3
80104744:  subu v0,v0,a0
80104748:  sll v0,v0,0x2
8010474c:  addu v0,t5,v0
80104750:  lhu a3,0x36(v0)
80104754:  lui t2,0x8006
80104758:  addiu t2,t2,0x7b4
8010475c:  andi v1,v1,0xfff
80104760:  sll v1,v1,0x2
80104764:  addu v1,v1,t2
80104768:  lh s8,0x2(v1)
8010476c:  ori v0,zero,0xffff
80104770:  lh s6,0x0(v1)
80104774:  beq a3,v0,0x801048ec
80104778:  _clear s7
8010477c:  addiu a0,sp,0x70
80104780:  clear a1
80104784:  lw t3,0x84(sp)
80104788:  li a2,0x10
8010478c:  sll v0,a3,0x3
80104790:  subu v0,v0,a3
80104794:  sll s5,v0,0x2
80104798:  addiu v0,s5,0x1c
8010479c:  jal 0x80044efc
801047a0:  _addu s0,t3,v0
801047a4:  lw v0,0x4(s0)
801047a8:  nop
801047ac:  mult s8,v0
801047b0:  addiu s1,s0,0x4
801047b4:  lw v0,0x8(s1)
801047b8:  mflo v1
801047bc:  nop
801047c0:  nop
801047c4:  mult s6,v0
801047c8:  mflo t0
801047cc:  addu v0,v1,t0
801047d0:  bgez v0,0x801047dc
801047d4:  _nop
801047d8:  addiu v0,v0,0xfff
801047dc:  sra v0,v0,0xc
801047e0:  sw v0,0x70(sp)
801047e4:  lw v1,0x4(s0)
801047e8:  subu v0,zero,s6
801047ec:  mult v0,v1
801047f0:  lw v0,0x8(s1)
801047f4:  mflo v1
801047f8:  nop
801047fc:  nop
80104800:  mult s8,v0
80104804:  mflo t0
80104808:  addu v0,v1,t0
8010480c:  bgez v0,0x80104818
80104810:  _nop
80104814:  addiu v0,v0,0xfff
80104818:  lw a0,0x88(sp)
8010481c:  lw a1,0x8c(sp)
80104820:  sra v0,v0,0xc
80104824:  sw v0,0x78(sp)
80104828:  lw t2,0x70(sp)
8010482c:  lw t3,0x74(sp)
80104830:  lw t4,0x78(sp)
80104834:  lw t5,0x7c(sp)
80104838:  sw t2,0x60(sp)
8010483c:  sw t3,0x64(sp)
80104840:  sw t4,0x68(sp)
80104844:  sw t5,0x6c(sp)
80104848:  jal 0x800171d4
8010484c:  _nop
80104850:  lw a0,0x88(sp)
80104854:  move s2,v0
80104858:  jal 0x80016a20
8010485c:  _move s3,v1
80104860:  lw a0,0x8c(sp)
80104864:  jal 0x80016a20
80104868:  _move s0,v0
8010486c:  bgez s0,0x80104878
80104870:  _move v1,v0
80104874:  addiu s0,s0,0xfff
80104878:  sra v0,s0,0xc
8010487c:  mult v0,v1
80104880:  move a0,s2
80104884:  move a1,s3
80104888:  mflo v0
8010488c:  move a2,v0
80104890:  jal 0x8004779c
80104894:  _sra a3,v0,0x1f
80104898:  move a0,v0
8010489c:  move a1,v1
801048a0:  slt v0,s7,a0
801048a4:  beq v0,zero,0x801048d0
801048a8:  _nop
801048ac:  move s7,a0
801048b0:  lw t2,0x60(sp)
801048b4:  lw t3,0x64(sp)
801048b8:  lw t4,0x68(sp)
801048bc:  lw t5,0x6c(sp)
801048c0:  sw t2,0x50(sp)
801048c4:  sw t3,0x54(sp)
801048c8:  sw t4,0x58(sp)
801048cc:  sw t5,0x5c(sp)
801048d0:  lw t2,0x84(sp)
801048d4:  nop
801048d8:  addu v0,t2,s5
801048dc:  lhu a3,0x34(v0)
801048e0:  ori v0,zero,0xffff
801048e4:  bne a3,v0,0x80104780
801048e8:  _addiu a0,sp,0x70
801048ec:  lw v0,0x0(s4)
801048f0:  lw t3,0x90(sp)
801048f4:  lw v1,0x50(sp)
801048f8:  addiu s0,sp,0x10
801048fc:  addu s0,s0,t3
80104900:  addu v0,v0,v1
80104904:  sw v0,0x0(s0)
80104908:  lw a1,0x8(s4)
8010490c:  lw v0,0x58(sp)
80104910:  lw a0,0x0(s0)
80104914:  addu a1,a1,v0
80104918:  jal 0x80025400
8010491c:  _sw a1,0x8(s0)
80104920:  slti a0,zero,0x1281
80104924:  sw v0,0x4(s0)
80104928:  lhu v0,0x10(s4)
8010492c:  nop
80104930:  andi v0,v0,0x1
80104934:  bne v0,zero,0x80104a04
80104938:  _li v0,0x2
8010493c:  lw t4,0xc0(sp)
80104940:  lw t5,0x80(sp)
80104944:  lhu v1,0xc(t4)
80104948:  sllv v0,v0,t5
8010494c:  and v1,v1,v0
80104950:  bne v1,zero,0x80104a04
80104954:  _clear a1
80104958:  lh v0,0x12(s4)
8010495c:  nop
80104960:  blez v0,0x801049f0
80104964:  _move a2,a1
80104968:  lhu t1,0x8(t4)
8010496c:  move t0,v0
80104970:  move a0,s4
80104974:  lui v0,0x8006
80104978:  lw a3,0x5bd4(v0)
8010497c:  lw v1,0x1c(a0)
80104980:  nop
80104984:  lhu v0,0x8(v1)
80104988:  nop
8010498c:  sltu v0,v0,t1
80104990:  beq v0,zero,0x801049e0
80104994:  _nop
80104998:  lhu v1,0xa(v1)
8010499c:  nop
801049a0:  sll v0,v1,0x1
801049a4:  addu v0,v0,v1
801049a8:  sll v0,v0,0x2
801049ac:  addu v0,v0,v1
801049b0:  sll v0,v0,0x2
801049b4:  addu v0,v0,a3
801049b8:  lw v0,0x24(v0)
801049bc:  nop
801049c0:  srl v1,v0,0x1f
801049c4:  addu v0,v0,v1
801049c8:  sra v1,v0,0x1
801049cc:  slt v0,v1,a2
801049d0:  beq v0,zero,0x801049dc
801049d4:  _nop
801049d8:  move v1,a2
801049dc:  move a2,v1
801049e0:  addiu a1,a1,0x1
801049e4:  slt v0,a1,t0
801049e8:  bne v0,zero,0x8010497c
801049ec:  _addiu a0,a0,0x4
801049f0:  beq a2,zero,0x80104a04
801049f4:  _nop
801049f8:  lw a1,0x80(sp)
801049fc:  sltiu a0,zero,0xe6f
80104a00:  addiu a0,sp,0x10
80104a04:  lw t2,0x90(sp)
80104a08:  lw t3,0x94(sp)
80104a0c:  lw t4,0x80(sp)
80104a10:  addiu t2,t2,0x24
80104a14:  addiu t3,t3,0x8
80104a18:  addiu t4,t4,0x1
80104a1c:  slti v0,t4,0x2
80104a20:  sw t2,0x90(sp)
80104a24:  sw t3,0x94(sp)
80104a28:  bne v0,zero,0x801046b8
80104a2c:  _sw t4,0x80(sp)
80104a30:  lw t5,0xc0(sp)
80104a34:  lui v0,0x8006
80104a38:  lw v0,0x5bd4(v0)
80104a3c:  lhu v1,0xa(t5)
80104a40:  addiu a0,sp,0x10
80104a44:  lhu a2,0xc(t5)
80104a48:  sll a1,v1,0x1
80104a4c:  addu a1,a1,v1
80104a50:  sll a1,a1,0x2
80104a54:  addu a1,a1,v1
80104a58:  sll a1,a1,0x2
80104a5c:  sltiu a0,zero,0xdaf
80104a60:  addu a1,v0,a1
80104a64:  lw ra,0xbc(sp)
80104a68:  lw s8,0xb8(sp)
80104a6c:  lw s7,0xb4(sp)
80104a70:  lw s6,0xb0(sp)
80104a74:  lw s5,0xac(sp)
80104a78:  lw s4,0xa8(sp)
80104a7c:  lw s3,0xa4(sp)
80104a80:  lw s2,0xa0(sp)
80104a84:  lw s1,0x9c(sp)
80104a88:  lw s0,0x98(sp)
80104a8c:  jr ra
80104a90:  _addiu sp,sp,0xc0
