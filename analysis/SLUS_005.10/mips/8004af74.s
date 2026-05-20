# addr: 0x8004af74  name: CDREAD_OBJ_260
8004af74:  addiu sp,sp,-0x18
8004af78:  sw s0,0x10(sp)
8004af7c:  lui s0,0x8006
8004af80:  addiu s0,s0,0x398
8004af84:  sw ra,0x14(sp)
8004af88:  lw v0,0x10(s0)
8004af8c:  lw v1,0x8(s0)
8004af90:  sll v0,v0,0x2
8004af94:  addu v1,v1,v0
8004af98:  sw v1,0x8(s0)
8004af9c:  lw v0,0x14(s0)
8004afa0:  nop
8004afa4:  addiu v0,v0,-0x1
8004afa8:  sw v0,0x14(s0)
8004afac:  lw v0,0x20(s0)
8004afb0:  nop
8004afb4:  addiu v0,v0,0x1
8004afb8:  sw v0,0x20(s0)
8004afbc:  lw v0,0x14(s0)
8004afc0:  nop
8004afc4:  bne v0,zero,0x8004b030
8004afc8:  _nop
8004afcc:  lw a0,0x24(s0)
8004afd0:  jal 0x80048fa8
8004afd4:  _nop
8004afd8:  lw a0,0x28(s0)
8004afdc:  jal 0x80048fbc
8004afe0:  _nop
8004afe4:  lw v0,0x30(s0)
8004afe8:  nop
8004afec:  andi v0,v0,0x1
8004aff0:  beq v0,zero,0x8004b008
8004aff4:  _li a0,0x9
8004aff8:  lw a0,0x2c(s0)
8004affc:  jal 0x800493ec
8004b000:  _nop
8004b004:  li a0,0x9
8004b008:  jal 0x8004910c
8004b00c:  _clear a1
8004b010:  lui v0,0x8006
8004b014:  lw v0,0x394(v0)
8004b018:  nop
8004b01c:  beq v0,zero,0x8004b030
8004b020:  _nop
8004b024:  lw a1,0x34(s0)
8004b028:  jalr v0
8004b02c:  _li a0,0x2
8004b030:  lw ra,0x14(sp)
8004b034:  lw s0,0x10(sp)
8004b038:  jr ra
8004b03c:  _addiu sp,sp,0x18
