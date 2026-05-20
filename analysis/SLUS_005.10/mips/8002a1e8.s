# addr: 0x8002a1e8  name: FUN_8002a1e8
8002a1e8:  lw v1,0x4(gp)
8002a1ec:  lui v0,0x800a
8002a1f0:  beq v1,zero,0x8002a1fc
8002a1f4:  _addiu t0,v0,0x1e20
8002a1f8:  addiu t0,t0,0x504
8002a1fc:  lw v1,0x0(t0)
8002a200:  nop
8002a204:  slti v0,v1,0x40
8002a208:  beq v0,zero,0x8002a228
8002a20c:  _sll v0,v1,0x2
8002a210:  addu v0,v0,v1
8002a214:  sll v0,v0,0x2
8002a218:  addiu v0,v0,0x4
8002a21c:  addu t1,t0,v0
8002a220:  addiu v0,v1,0x1
8002a224:  sw v0,0x0(t0)
8002a228:  sw a0,0x8(t1)
8002a22c:  sw a3,0xc(t1)
8002a230:  sw a2,0x10(t1)
8002a234:  lw v0,0x0(a1)
8002a238:  sll v1,t1,0x8
8002a23c:  srl v1,v1,0x8
8002a240:  sw v1,0x0(a1)
8002a244:  lbu v1,0x3(t1)
8002a248:  nop
8002a24c:  sll v1,v1,0x18
8002a250:  or v1,v1,v0
8002a254:  jr ra
8002a258:  _sw v1,0x0(t1)
