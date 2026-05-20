# addr: 0x8002bc18  name: FUN_8002bc18
8002bc18:  addiu sp,sp,-0x40
8002bc1c:  sw s1,0x34(sp)
8002bc20:  move s1,a0
8002bc24:  sw ra,0x38(sp)
8002bc28:  jal 0x8003fbc8
8002bc2c:  _sw s0,0x30(sp)
8002bc30:  move a2,v0
8002bc34:  beq a2,zero,0x8002bc90
8002bc38:  _move a1,s1
8002bc3c:  lw v0,0x58(s1)
8002bc40:  addiu a0,sp,0x10
8002bc44:  lw v1,0x0(v0)
8002bc48:  sll v0,a2,0x3
8002bc4c:  subu v0,v0,a2
8002bc50:  sll v0,v0,0x2
8002bc54:  addiu v0,v0,0x1c
8002bc58:  addu s0,v1,v0
8002bc5c:  jal 0x8001d68c
8002bc60:  _move a2,s0
8002bc64:  lw a0,0x58(s1)
8002bc68:  lhu a1,0x1a(s0)
8002bc6c:  jal 0x8003f4a0
8002bc70:  _move a2,v0
8002bc74:  lhu v1,0x16(s0)
8002bc78:  ori v0,zero,0xaaaa
8002bc7c:  beq v1,v0,0x8002bc88
8002bc80:  _clear a1
8002bc84:  move a1,v1
8002bc88:  j 0x8002bc98
8002bc8c:  _move a0,s1
8002bc90:  move a0,s1
8002bc94:  clear a1
8002bc98:  jal 0x80020890
8002bc9c:  _nop
8002bca0:  lb a0,0x5(s1)
8002bca4:  lw v1,0x0(s1)
8002bca8:  lui v0,0x8003
8002bcac:  addiu v0,v0,-0x4674
8002bcb0:  sw v0,0x64(s1)
8002bcb4:  li v0,-0x4001
8002bcb8:  sh zero,0xc(s1)
8002bcbc:  sh zero,0xa6(s1)
8002bcc0:  sh zero,0xbe(s1)
8002bcc4:  ori v1,v1,0x8000
8002bcc8:  and v1,v1,v0
8002bccc:  jal 0x800441c8
8002bcd0:  _sw v1,0x0(s1)
8002bcd4:  lbu a0,0xd3(s1)
8002bcd8:  jal 0x800441c8
8002bcdc:  _nop
8002bce0:  jal 0x80042f5c
8002bce4:  _addiu a0,s1,0xc0
8002bce8:  lh v0,0x6(s1)
8002bcec:  nop
8002bcf0:  blez v0,0x8002bd50
8002bcf4:  _li v0,0x1
8002bcf8:  lbu v0,0x67c(gp)
8002bcfc:  lw a0,0x7d0(gp)
8002bd00:  sh zero,0x6(s1)
8002bd04:  lw v1,0xe4(a0)
8002bd08:  addiu v0,v0,0x1
8002bd0c:  sb v0,0x67c(gp)
8002bd10:  bne v1,s1,0x8002bd20
8002bd14:  _nop
8002bd18:  jal 0x8002ea94
8002bd1c:  _li a1,0x1
8002bd20:  lw a0,0x7d4(gp)
8002bd24:  nop
8002bd28:  beq a0,zero,0x8002bd70
8002bd2c:  _nop
8002bd30:  lw v0,0xe4(a0)
8002bd34:  nop
8002bd38:  bne v0,s1,0x8002bd70
8002bd3c:  _nop
8002bd40:  jal 0x8002ea94
8002bd44:  _li a1,0x1
8002bd48:  j 0x8002bd70
8002bd4c:  _nop
8002bd50:  lw a0,0xe0(s1)
8002bd54:  nop
8002bd58:  lw v1,0x0(a0)
8002bd5c:  sw v0,0x624(gp)
8002bd60:  lui v0,0x2
8002bd64:  or v1,v1,v0
8002bd68:  sw v1,0x0(a0)
8002bd6c:  sw zero,0xe4(s1)
8002bd70:  lw ra,0x38(sp)
8002bd74:  lw s1,0x34(sp)
8002bd78:  lw s0,0x30(sp)
8002bd7c:  jr ra
8002bd80:  _addiu sp,sp,0x40
