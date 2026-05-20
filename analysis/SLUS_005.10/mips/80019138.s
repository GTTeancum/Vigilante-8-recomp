# addr: 0x80019138  name: FUN_80019138
80019138:  lbu v1,0x0(a1)
8001913c:  move t1,a0
80019140:  clear a3
80019144:  move a0,a3
80019148:  beq v1,zero,0x800191b4
8001914c:  _addiu a1,a1,0x1
80019150:  lui v0,0x8005
80019154:  addiu t0,v0,0x68dc
80019158:  andi a2,v1,0xff
8001915c:  sltiu v0,a2,0x20
80019160:  bne v0,zero,0x80019198
80019164:  _addu v0,a2,t0
80019168:  lw a0,0x0(t1)
8001916c:  nop
80019170:  lbu v1,0x5(a0)
80019174:  nop
80019178:  subu v1,a2,v1
8001917c:  sll v0,v1,0x2
80019180:  addu v0,v0,v1
80019184:  addiu v0,v0,0x8
80019188:  addu a0,a0,v0
8001918c:  lbu v0,0x3(a0)
80019190:  j 0x800191a4
80019194:  _addu a3,a3,v0
80019198:  lbu v0,0x0(v0)
8001919c:  nop
800191a0:  addu a1,a1,v0
800191a4:  lbu v1,0x0(a1)
800191a8:  nop
800191ac:  bne v1,zero,0x80019158
800191b0:  _addiu a1,a1,0x1
800191b4:  beq a0,zero,0x800191d8
800191b8:  _nop
800191bc:  lbu v0,0x3(a0)
800191c0:  lb v1,0x4(a0)
800191c4:  lbu a0,0x2(a0)
800191c8:  subu v0,a3,v0
800191cc:  addu v0,v0,v1
800191d0:  jr ra
800191d4:  _addu v0,v0,a0
800191d8:  jr ra
800191dc:  _move v0,a3
