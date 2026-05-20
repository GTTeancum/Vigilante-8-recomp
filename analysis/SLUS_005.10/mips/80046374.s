# addr: 0x80046374  name: _spu_gcSPU
80046374:  lui v0,0x8006
80046378:  lw v0,-0x11c8(v0)
8004637c:  nop
80046380:  bltz v0,0x80046450
80046384:  _clear t1
80046388:  lui t4,0x8000
8004638c:  lui t2,0x2fff
80046390:  ori t2,t2,0xffff
80046394:  lui t3,0xfff
80046398:  ori t3,t3,0xffff
8004639c:  lui t0,0x8006
800463a0:  lw t0,-0x11c4(t0)
800463a4:  move t5,v0
800463a8:  move a3,t0
800463ac:  lw v0,0x0(a3)
800463b0:  nop
800463b4:  and v0,v0,t4
800463b8:  beq v0,zero,0x80046430
800463bc:  _addiu a2,t1,0x1
800463c0:  sll v0,a2,0x3
800463c4:  addu v1,v0,t0
