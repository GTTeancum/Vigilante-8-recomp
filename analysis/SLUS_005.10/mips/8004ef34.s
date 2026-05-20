# addr: 0x8004ef34  name: LoadTPage
8004ef34:  addiu sp,sp,-0x30
8004ef38:  sw s3,0x24(sp)
8004ef3c:  lw s3,0x40(sp)
8004ef40:  lw v1,0x44(sp)
8004ef44:  lw v0,0x48(sp)
8004ef48:  move t0,a0
8004ef4c:  sw s0,0x18(sp)
8004ef50:  move s0,a1
8004ef54:  sw s2,0x20(sp)
8004ef58:  move s2,a2
8004ef5c:  sw s1,0x1c(sp)
8004ef60:  move s1,a3
8004ef64:  sw ra,0x28(sp)
8004ef68:  sh s1,0x10(sp)
8004ef6c:  sh v0,0x16(sp)
8004ef70:  li v0,0x1
8004ef74:  beq s0,v0,0x8004efc4
8004ef78:  _sh s3,0x12(sp)
8004ef7c:  slti v0,s0,0x2
8004ef80:  beq v0,zero,0x8004ef98
8004ef84:  _nop
8004ef88:  beq s0,zero,0x8004efac
8004ef8c:  _addiu a0,sp,0x10
8004ef90:  j 0x8004efe0
8004ef94:  _nop
8004ef98:  li v0,0x2
8004ef9c:  beq s0,v0,0x8004efd8
8004efa0:  _addiu a0,sp,0x10
8004efa4:  j 0x8004efe0
8004efa8:  _nop
8004efac:  bgez v1,0x8004efb8
8004efb0:  _move v0,v1
8004efb4:  addiu v0,v1,0x3
8004efb8:  sra v0,v0,0x2
8004efbc:  j 0x8004efdc
8004efc0:  _sh v0,0x14(sp)
8004efc4:  srl v0,v1,0x1f
8004efc8:  addu v0,v1,v0
8004efcc:  sra v0,v0,0x1
8004efd0:  j 0x8004efdc
8004efd4:  _sh v0,0x14(sp)
8004efd8:  sh v1,0x14(sp)
