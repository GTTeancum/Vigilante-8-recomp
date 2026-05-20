# addr: 0x8001bda0  name: FUN_8001bda0
8001bda0:  addiu sp,sp,-0x18
8001bda4:  sw ra,0x10(sp)
8001bda8:  lw v1,0x0(a0)
8001bdac:  andi a1,a1,0xffff
8001bdb0:  sll v0,a1,0x3
8001bdb4:  subu v0,v0,a1
8001bdb8:  sll v0,v0,0x2
8001bdbc:  addu v1,v1,v0
8001bdc0:  lhu a1,0x1c(v1)
8001bdc4:  jal 0x8001b49c
8001bdc8:  _andi a1,a1,0x7ff
8001bdcc:  lw ra,0x10(sp)
8001bdd0:  nop
8001bdd4:  jr ra
8001bdd8:  _addiu sp,sp,0x18
