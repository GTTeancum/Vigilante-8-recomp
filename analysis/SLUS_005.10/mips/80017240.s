# addr: 0x80017240  name: FUN_80017240
80017240:  lw v0,0x0(a1)
80017244:  lw v1,0x0(a0)
80017248:  sll t0,v0,0x10
8001724c:  sra t0,t0,0x10
80017250:  mult v1,t0
80017254:  lw v1,0x4(a0)
80017258:  mfhi t1
8001725c:  mflo t0
80017260:  sra v0,v0,0x10
80017264:  nop
80017268:  mult v1,v0
8001726c:  lw v1,0x8(a0)
80017270:  lh v0,0x4(a1)
80017274:  mfhi a3
80017278:  mflo a2
8001727c:  nop
80017280:  nop
80017284:  mult v1,v0
80017288:  addu v0,t0,a2
8001728c:  sltu a0,v0,a2
80017290:  addu v1,t1,a3
80017294:  addu v1,v1,a0
80017298:  mfhi t3
8001729c:  mflo t2
800172a0:  addu v0,v0,t2
800172a4:  sltu a0,v0,t2
800172a8:  addu v1,v1,t3
800172ac:  jr ra
800172b0:  _addu v1,v1,a0
