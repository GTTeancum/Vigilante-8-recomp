# addr: 0x800171d4  name: FUN_800171d4
800171d4:  lw v1,0x0(a0)
800171d8:  lw v0,0x0(a1)
800171dc:  nop
800171e0:  mult v1,v0
800171e4:  lw v1,0x4(a0)
800171e8:  mfhi t1
800171ec:  mflo t0
800171f0:  lw v0,0x4(a1)
800171f4:  nop
800171f8:  mult v1,v0
800171fc:  lw v1,0x8(a0)
80017200:  mfhi a3
80017204:  mflo a2
80017208:  lw v0,0x8(a1)
8001720c:  nop
80017210:  mult v1,v0
80017214:  addu v0,t0,a2
80017218:  sltu a0,v0,a2
8001721c:  addu v1,t1,a3
80017220:  addu v1,v1,a0
80017224:  mfhi t3
80017228:  mflo t2
8001722c:  addu v0,v0,t2
80017230:  sltu a0,v0,t2
80017234:  addu v1,v1,t3
80017238:  jr ra
8001723c:  _addu v1,v1,a0
