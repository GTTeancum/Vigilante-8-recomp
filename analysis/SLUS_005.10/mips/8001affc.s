# addr: 0x8001affc  name: FUN_8001affc
8001affc:  addiu sp,sp,-0x18
8001b000:  sw ra,0x10(sp)
8001b004:  lw a0,0x0(a0)
8001b008:  andi a1,a1,0xffff
8001b00c:  sll v0,a1,0x3
8001b010:  subu v0,v0,a1
8001b014:  sll v0,v0,0x2
8001b018:  addu v0,a0,v0
8001b01c:  lhu a1,0x36(v0)
8001b020:  jal 0x8001afa0
8001b024:  _andi a2,a2,0xffff
8001b028:  lw ra,0x10(sp)
8001b02c:  nop
8001b030:  jr ra
8001b034:  _addiu sp,sp,0x18
