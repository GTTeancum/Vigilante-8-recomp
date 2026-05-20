# addr: 0x80024888  name: FUN_80024888
80024888:  lw v1,0x7ec(gp)
8002488c:  nop
80024890:  beq v1,zero,0x800248a4
80024894:  _move t3,v1
80024898:  lw v0,0x0(v1)
8002489c:  nop
800248a0:  sw v0,0x7ec(gp)
800248a4:  beq t3,zero,0x80024990
800248a8:  _nop
800248ac:  lbu t1,0x11(a0)
800248b0:  lhu v0,0xc(a0)
800248b4:  lhu v1,0xe(a0)
800248b8:  lw a0,0x8(a0)
800248bc:  xor v0,v0,a1
800248c0:  xor v1,v1,a2
800248c4:  or v0,v0,v1
800248c8:  srav t0,v0,t1
800248cc:  sra t0,t0,0x1
800248d0:  beq t0,zero,0x8002490c
800248d4:  _addiu t1,t1,0x1
800248d8:  lhu v0,0x0(a0)
800248dc:  nop
800248e0:  bne v0,zero,0x800248f0
800248e4:  _addiu t1,t1,0x1
800248e8:  jr ra
800248ec:  _clear v0
800248f0:  lhu v1,0x0(a0)
800248f4:  sra t0,t0,0x1
800248f8:  sll v0,v1,0x2
800248fc:  addu v0,v0,v1
80024900:  sll v0,v0,0x1
80024904:  bne t0,zero,0x800248d8
80024908:  _subu a0,a0,v0
8002490c:  li v0,0x20
80024910:  subu v0,v0,t1
80024914:  sllv t0,a1,v0
80024918:  sllv t2,a2,v0
8002491c:  addiu t1,t1,-0x1
80024920:  bgez t2,0x8002492c
80024924:  _srl a3,t0,0x1f
80024928:  ori a3,a3,0x2
8002492c:  sll v0,a3,0x1
80024930:  addu v0,a0,v0
80024934:  lhu v0,0x2(v0)
80024938:  nop
8002493c:  andi v1,v0,0xffff
80024940:  beq v1,zero,0x8002496c
80024944:  _nop
80024948:  andi v0,v0,0x8000
8002494c:  bne v0,zero,0x8002496c
80024950:  _sll t0,t0,0x1
80024954:  sll t2,t2,0x1
80024958:  sll v0,v1,0x2
8002495c:  addu v0,v0,v1
80024960:  sll v0,v0,0x1
80024964:  j 0x8002491c
80024968:  _addu a0,a0,v0
8002496c:  li v0,-0x1
80024970:  sllv v0,v0,t1
80024974:  and v1,a1,v0
80024978:  and v0,a2,v0
8002497c:  sw a0,0x8(t3)
80024980:  sb a3,0x10(t3)
80024984:  sb t1,0x11(t3)
80024988:  sh v1,0xc(t3)
8002498c:  sh v0,0xe(t3)
80024990:  jr ra
80024994:  _move v0,t3
