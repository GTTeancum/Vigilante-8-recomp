# addr: 0x800244c4  name: FUN_800244c4
800244c4:  lw a3,0x6ec(gp)
800244c8:  sll a0,a0,0x5
800244cc:  sll a1,a1,0x5
800244d0:  bgez a1,0x800244dc
800244d4:  _srl v0,a0,0x1f
800244d8:  ori v0,v0,0x2
800244dc:  sll v0,v0,0x1
800244e0:  addu v0,a3,v0
800244e4:  lhu a2,0x2(v0)
800244e8:  nop
800244ec:  andi v1,a2,0xffff
800244f0:  beq v1,zero,0x80024518
800244f4:  _andi v0,a2,0x8000
800244f8:  bne v0,zero,0x80024518
800244fc:  _sll v0,v1,0x2
80024500:  addu v0,v0,v1
80024504:  sll v0,v0,0x1
80024508:  addu a3,a3,v0
8002450c:  sll a0,a0,0x1
80024510:  j 0x800244d0
80024514:  _sll a1,a1,0x1
80024518:  jr ra
8002451c:  _move v0,a2
