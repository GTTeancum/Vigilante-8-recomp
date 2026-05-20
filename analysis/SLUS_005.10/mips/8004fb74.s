# addr: 0x8004fb74  name: DrawOTag
8004fb74:  lui v0,0x8006
8004fb78:  lbu v0,0x5026(v0)
8004fb7c:  addiu sp,sp,-0x18
8004fb80:  sw s0,0x10(sp)
8004fb84:  move s0,a0
8004fb88:  sltiu v0,v0,0x2
8004fb8c:  bne v0,zero,0x8004fbb0
8004fb90:  _sw ra,0x14(sp)
8004fb94:  lui a0,0x8001
8004fb98:  addiu a0,a0,0x12e0
8004fb9c:  lui v0,0x8006
8004fba0:  lw v0,0x5020(v0)
8004fba4:  nop
8004fba8:  jalr v0
8004fbac:  _move a1,s0
8004fbb0:  move a1,s0
8004fbb4:  lui v0,0x8006
8004fbb8:  lw v0,0x501c(v0)
8004fbbc:  clear a2
8004fbc0:  lw a0,0x18(v0)
8004fbc4:  lw v0,0x8(v0)
8004fbc8:  nop
8004fbcc:  jalr v0
8004fbd0:  _clear a3
8004fbd4:  lw ra,0x14(sp)
8004fbd8:  lw s0,0x10(sp)
8004fbdc:  jr ra
8004fbe0:  _addiu sp,sp,0x18
