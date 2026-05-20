# addr: 0x8002036c  name: FUN_8002036c
8002036c:  addiu sp,sp,-0x18
80020370:  sw s0,0x10(sp)
80020374:  sw ra,0x14(sp)
80020378:  jal 0x8001d708
8002037c:  _move s0,a0
80020380:  jal 0x8001dc1c
80020384:  _move a0,s0
80020388:  lw v0,0x64(s0)
8002038c:  nop
80020390:  beq v0,zero,0x800203ac
80020394:  _move a0,s0
80020398:  li a1,0x1
8002039c:  jalr v0
800203a0:  _clear a2
800203a4:  j 0x800203b0
800203a8:  _nop
800203ac:  clear v0
800203b0:  bltz v0,0x800203ec
800203b4:  _clear v0
800203b8:  lw v0,0x0(s0)
800203bc:  nop
800203c0:  andi v0,v0,0x8
800203c4:  beq v0,zero,0x800203e4
800203c8:  _nop
800203cc:  lw v0,0x70(s0)
800203d0:  nop
800203d4:  bne v0,zero,0x800203e4
800203d8:  _nop
800203dc:  jal 0x8003e730
800203e0:  _move a0,s0
800203e4:  jal 0x800202f4
800203e8:  _move a0,s0
800203ec:  lw ra,0x14(sp)
800203f0:  lw s0,0x10(sp)
800203f4:  jr ra
800203f8:  _addiu sp,sp,0x18
