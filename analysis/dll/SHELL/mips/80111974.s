# addr: 0x80111974  name: FUN_80111974
80111974:  lui v0,0x8011
80111978:  lw v0,0x3378(v0)
8011197c:  addiu sp,sp,-0x10
80111980:  sh zero,0xa(v0)
80111984:  li v0,0xa
80111988:  sw v0,0x0(sp)
8011198c:  lw v0,0x0(sp)
80111990:  nop
80111994:  addiu v0,v0,-0x1
80111998:  sw v0,0x0(sp)
8011199c:  lw v1,0x0(sp)
801119a0:  li v0,-0x1
801119a4:  beq v1,v0,0x801119d0
801119a8:  _clear v0
801119ac:  li v1,-0x1
801119b0:  lw v0,0x0(sp)
801119b4:  nop
801119b8:  addiu v0,v0,-0x1
801119bc:  sw v0,0x0(sp)
801119c0:  lw v0,0x0(sp)
801119c4:  nop
801119c8:  bne v0,v1,0x801119b0
801119cc:  _clear v0
801119d0:  addiu sp,sp,0x10
801119d4:  jr ra
801119d8:  _nop
