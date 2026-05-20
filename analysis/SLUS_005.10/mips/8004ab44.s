# addr: 0x8004ab44  name: CD_getsector2
8004ab44:  lui v0,0x8006
8004ab48:  lw v0,0x344(v0)
8004ab4c:  lui a2,0x2102
8004ab50:  sb zero,0x0(v0)
8004ab54:  lui v1,0x8006
8004ab58:  lw v1,0x350(v1)
8004ab5c:  li v0,0x80
8004ab60:  sb v0,0x0(v1)
8004ab64:  lui v0,0x8006
8004ab68:  lw v0,0x378(v0)
8004ab6c:  ori a2,a2,0x843
8004ab70:  sw a2,0x0(v0)
8004ab74:  lui v1,0x8006
8004ab78:  lw v1,0x354(v1)
8004ab7c:  li v0,0x1325
8004ab80:  sw v0,0x0(v1)
8004ab84:  lui v1,0x8006
8004ab88:  lw v1,0x37c(v1)
8004ab8c:  nop
8004ab90:  lw v0,0x0(v1)
8004ab94:  nop
8004ab98:  ori v0,v0,0x8000
8004ab9c:  sw v0,0x0(v1)
8004aba0:  lui v0,0x8006
8004aba4:  lw v0,0x380(v0)
8004aba8:  nop
8004abac:  sw a0,0x0(v0)
8004abb0:  lui v0,0x1
8004abb4:  lui v1,0x8006
8004abb8:  lw v1,0x384(v1)
8004abbc:  or a1,a1,v0
8004abc0:  sw a1,0x0(v1)
8004abc4:  lui v1,0x8006
8004abc8:  lw v1,0x344(v1)
8004abcc:  nop
8004abd0:  lbu v0,0x0(v1)
8004abd4:  nop
8004abd8:  andi v0,v0,0x40
8004abdc:  bne v0,zero,0x8004abf8
8004abe0:  _addiu sp,sp,-0x8
8004abe4:  lbu v0,0x0(v1)
8004abe8:  nop
8004abec:  andi v0,v0,0x40
8004abf0:  beq v0,zero,0x8004abe4
8004abf4:  _nop
8004abf8:  lui v1,0x1140
8004abfc:  lui v0,0x8006
8004ac00:  lw v0,0x388(v0)
8004ac04:  ori v1,v1,0x100
8004ac08:  sw v1,0x0(v0)
8004ac0c:  lui v0,0x8006
8004ac10:  lw v0,0x388(v0)
8004ac14:  nop
8004ac18:  lw v0,0x0(v0)
8004ac1c:  nop
8004ac20:  sw v0,0x0(sp)
8004ac24:  clear v0
8004ac28:  jr ra
8004ac2c:  _addiu sp,sp,0x8
