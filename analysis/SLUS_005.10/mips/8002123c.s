# addr: 0x8002123c  name: FUN_8002123c
8002123c:  addiu sp,sp,-0x20
80021240:  sw s0,0x10(sp)
80021244:  move s0,a0
80021248:  sw ra,0x1c(sp)
8002124c:  sw s2,0x18(sp)
80021250:  sw s1,0x14(sp)
80021254:  lw v0,0x0(s0)
80021258:  move s1,a1
8002125c:  bne v0,zero,0x80021274
80021260:  _move s2,a2
80021264:  jal 0x800200b8
80021268:  _addiu a0,s0,0x4
8002126c:  j 0x800212ac
80021270:  _nop
80021274:  sltiu v0,v0,0x3
80021278:  beq v0,zero,0x800212ac
8002127c:  _move a1,s1
80021280:  lw a0,0x8(s0)
80021284:  jal 0x8002123c
80021288:  _move a2,s2
8002128c:  bne v0,zero,0x800212ac
80021290:  _move a1,s1
80021294:  lw a0,0xc(s0)
80021298:  jal 0x8002123c
8002129c:  _move a2,s2
800212a0:  bne v0,zero,0x800212ac
800212a4:  _nop
800212a8:  clear v0
800212ac:  lw ra,0x1c(sp)
800212b0:  lw s2,0x18(sp)
800212b4:  lw s1,0x14(sp)
800212b8:  lw s0,0x10(sp)
800212bc:  jr ra
800212c0:  _addiu sp,sp,0x20
