# addr: 0x80048490  name: INTR_OBJ_43C
80048490:  addiu sp,sp,-0x28
80048494:  sw s1,0x14(sp)
80048498:  move s1,a0
8004849c:  sw s2,0x18(sp)
800484a0:  move s2,a1
800484a4:  lui a1,0x8006
800484a8:  addiu a1,a1,-0x1110
800484ac:  sll v0,s1,0x2
800484b0:  addu a0,v0,a1
800484b4:  sw ra,0x24(sp)
800484b8:  sw s4,0x20(sp)
800484bc:  sw s3,0x1c(sp)
800484c0:  sw s0,0x10(sp)
800484c4:  lw s4,0x0(a0)
800484c8:  nop
800484cc:  beq s2,s4,0x800485b8
800484d0:  _move v0,s4
800484d4:  lhu v0,-0x4(a1)
800484d8:  nop
800484dc:  beq v0,zero,0x800485b4
800484e0:  _addiu a2,a1,-0x4
800484e4:  lui v0,0x8006
800484e8:  lw v0,-0x84(v0)
800484ec:  nop
800484f0:  lhu v1,0x0(v0)
800484f4:  sh zero,0x0(v0)
800484f8:  beq s2,zero,0x80048520
800484fc:  _andi s3,v1,0xffff
80048500:  li v1,0x1
80048504:  sllv v1,v1,s1
80048508:  sw s2,0x0(a0)
8004850c:  lhu v0,0x30(a2)
80048510:  or s3,s3,v1
80048514:  or v0,v0,v1
80048518:  j 0x80048540
8004851c:  _sh v0,0x30(a2)
80048520:  li v0,0x1
80048524:  sllv v0,v0,s1
80048528:  nor v0,zero,v0
8004852c:  sw zero,0x0(a0)
80048530:  lhu v1,0x2c(a1)
80048534:  and s3,s3,v0
80048538:  and v1,v1,v0
8004853c:  sh v1,0x2c(a1)
