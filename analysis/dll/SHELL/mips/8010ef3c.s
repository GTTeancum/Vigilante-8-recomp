# addr: 0x8010ef3c  name: FUN_8010ef3c
8010ef3c:  addiu sp,sp,-0x18
8010ef40:  lui v1,0x8011
8010ef44:  addiu v1,v1,0x3438
8010ef48:  sw ra,0x10(sp)
8010ef4c:  lw v0,0x0(v1)
8010ef50:  nop
8010ef54:  bne v0,zero,0x8010ef84
8010ef58:  _move a1,a0
8010ef5c:  lui a0,0x8011
8010ef60:  addiu a0,a0,-0x105c
8010ef64:  li v0,0x1
8010ef68:  sw v0,0x0(v1)
8010ef6c:  sw zero,0x4(v1)
8010ef70:  sw zero,0x8(v1)
8010ef74:  sltiu a0,zero,0x4300
8010ef78:  sw a1,0xc(v1)
8010ef7c:  slti a0,zero,0x3be5
8010ef80:  li v0,0x1
8010ef84:  lui a0,0x8010
8010ef88:  jal 0x80052604
8010ef8c:  _addiu a0,a0,0x10e4
8010ef90:  clear v0
8010ef94:  lw ra,0x10(sp)
8010ef98:  addiu sp,sp,0x18
8010ef9c:  jr ra
8010efa0:  _nop
