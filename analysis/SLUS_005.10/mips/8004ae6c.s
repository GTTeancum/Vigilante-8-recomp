# addr: 0x8004ae6c  name: CDREAD_OBJ_158
8004ae6c:  jal 0x80047e44
8004ae70:  _li a0,-0x1
8004ae74:  lui s0,0x8006
8004ae78:  addiu s0,s0,0x398
8004ae7c:  sw v0,0x18(s0)
8004ae80:  lw v0,0x14(s0)
8004ae84:  nop
8004ae88:  bgez v0,0x8004ae98
8004ae8c:  _nop
8004ae90:  jal 0x8004b040
8004ae94:  _li a0,0x1
8004ae98:  jal 0x80047e44
8004ae9c:  _li a0,-0x1
8004aea0:  lw v1,0x1c(s0)
8004aea4:  nop
8004aea8:  addiu v1,v1,0x4b0
8004aeac:  slt v1,v1,v0
8004aeb0:  beq v1,zero,0x8004aebc
8004aeb4:  _li v0,-0x1
8004aeb8:  sw v0,0x14(s0)
8004aebc:  lw v0,0x14(s0)
8004aec0:  nop
8004aec4:  beq v0,zero,0x8004aeec
8004aec8:  _nop
8004aecc:  jal 0x80047e44
8004aed0:  _li a0,-0x1
8004aed4:  lw v1,0x1c(s0)
8004aed8:  nop
8004aedc:  addiu v1,v1,0x4b0
8004aee0:  slt v1,v1,v0
8004aee4:  beq v1,zero,0x8004af60
8004aee8:  _nop
8004aeec:  lw a0,0x24(s0)
8004aef0:  jal 0x80048fa8
8004aef4:  _nop
8004aef8:  lw a0,0x28(s0)
8004aefc:  jal 0x80048fbc
8004af00:  _nop
8004af04:  lw v0,0x30(s0)
8004af08:  nop
8004af0c:  andi v0,v0,0x1
8004af10:  beq v0,zero,0x8004af28
8004af14:  _li a0,0x9
8004af18:  lw a0,0x2c(s0)
8004af1c:  jal 0x800493ec
8004af20:  _nop
8004af24:  li a0,0x9
8004af28:  jal 0x8004910c
8004af2c:  _clear a1
8004af30:  lui v1,0x8006
8004af34:  lw v1,0x394(v1)
8004af38:  nop
8004af3c:  beq v1,zero,0x8004af60
8004af40:  _nop
8004af44:  lw v0,0x14(s0)
8004af48:  nop
8004af4c:  bne v0,zero,0x8004af58
8004af50:  _li a0,0x5
8004af54:  li a0,0x2
8004af58:  jalr v1
8004af5c:  _move a1,s1
8004af60:  lw ra,0x28(sp)
8004af64:  lw s1,0x24(sp)
8004af68:  lw s0,0x20(sp)
8004af6c:  jr ra
8004af70:  _addiu sp,sp,0x30
