# addr: 0x80100168  name: FUN_80100168
80100168:  addiu sp,sp,-0x18
8010016c:  sw s0,0x10(sp)
80100170:  move s0,a0
80100174:  li v0,0x1
80100178:  beq a1,v0,0x801001a4
8010017c:  _sw ra,0x14(sp)
80100180:  li v0,0x5
80100184:  bne a1,v0,0x801001f8
80100188:  _lui v0,0x8006
8010018c:  lw a0,0x58fc(v0)
80100190:  li a1,0x3b
80100194:  jal 0x80044ac8
80100198:  _addiu a2,s0,0x48
8010019c:  slti a0,zero,0x7e
801001a0:  nop
801001a4:  lb v0,0x8(s0)
801001a8:  nop
801001ac:  beq v0,zero,0x801001f0
801001b0:  _lui v0,0x8002
801001b4:  lw a0,0x30(s0)
801001b8:  jal 0x8001bddc
801001bc:  _nop
801001c0:  lui v0,0x8007
801001c4:  sw zero,0x30(s0)
801001c8:  lw a1,0x37d8(v0)
801001cc:  move a0,s0
801001d0:  li a2,0x24
801001d4:  jal 0x8001add0
801001d8:  _li a3,0x8
801001dc:  lw v0,0x0(s0)
801001e0:  nop
801001e4:  ori v0,v0,0x34
801001e8:  slti a0,zero,0x7e
801001ec:  sw v0,0x0(s0)
801001f0:  addiu v0,v0,0x23dc
801001f4:  sw v0,0x64(s0)
801001f8:  lw ra,0x14(sp)
801001fc:  lw s0,0x10(sp)
80100200:  clear v0
80100204:  jr ra
80100208:  _addiu sp,sp,0x18
