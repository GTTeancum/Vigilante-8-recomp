# addr: 0x80030a88  name: FUN_80030a88
80030a88:  lh v1,0x0(a1)
80030a8c:  li v0,0x3
80030a90:  lw a3,0x8(a1)
80030a94:  beq v1,v0,0x80030b60
80030a98:  _addiu a2,a0,0xa4
80030a9c:  slti v0,v1,0x4
80030aa0:  beq v0,zero,0x80030ab8
80030aa4:  _li v0,0x2
80030aa8:  beq v1,v0,0x80030acc
80030aac:  _srl v1,a3,0x8
80030ab0:  jr ra
80030ab4:  _nop
80030ab8:  slti v0,v1,0x6
80030abc:  beq v0,zero,0x80030c00
80030ac0:  _nop
80030ac4:  j 0x80030b9c
80030ac8:  _nop
80030acc:  andi v0,a3,0x200
80030ad0:  beq v0,zero,0x80030ae4
80030ad4:  _andi v1,v1,0x1
80030ad8:  addiu v0,v1,-0x1
80030adc:  j 0x80030ae8
80030ae0:  _sll v0,v0,0x7
80030ae4:  sll v0,v1,0x7
80030ae8:  sh v0,0x2(a2)
80030aec:  andi v0,a3,0x800
80030af0:  beq v0,zero,0x80030b18
80030af4:  _andi v0,a3,0x1000
80030af8:  lh v0,0x0(a2)
80030afc:  lhu v1,0x0(a2)
80030b00:  slti v0,v0,-0x2a9
80030b04:  bne v0,zero,0x80030b18
80030b08:  _andi v0,a3,0x1000
80030b0c:  addiu v0,v1,-0x10
80030b10:  jr ra
80030b14:  _sh v0,0x0(a2)
80030b18:  beq v0,zero,0x80030b3c
80030b1c:  _nop
80030b20:  lh v0,0x0(a2)
80030b24:  lhu v1,0x0(a2)
80030b28:  slti v0,v0,0x2aa
80030b2c:  beq v0,zero,0x80030b3c
80030b30:  _addiu v0,v1,0x10
80030b34:  jr ra
80030b38:  _sh v0,0x0(a2)
80030b3c:  lh v0,0x0(a2)
80030b40:  lhu v1,0x0(a2)
80030b44:  bgez v0,0x80030b50
80030b48:  _nop
80030b4c:  addiu v0,v0,0xf
80030b50:  sra v0,v0,0x4
80030b54:  subu v0,v1,v0
80030b58:  jr ra
80030b5c:  _sh v0,0x0(a2)
80030b60:  lbu v1,0x10(a1)
80030b64:  nop
80030b68:  addiu v1,v1,-0x80
80030b6c:  sll v0,v1,0x2
80030b70:  addu v0,v0,v1
80030b74:  sh v0,0xa4(a0)
80030b78:  lbu v0,0x11(a1)
80030b7c:  lbu v1,0x12(a1)
80030b80:  nop
80030b84:  subu v0,v0,v1
80030b88:  srl v1,v0,0x1f
80030b8c:  addu v0,v0,v1
80030b90:  sra v0,v0,0x1
80030b94:  jr ra
80030b98:  _sh v0,0x2(a2)
80030b9c:  lbu v0,0x10(a1)
80030ba0:  nop
80030ba4:  addiu v0,v0,-0x80
80030ba8:  bgez v0,0x80030bb4
80030bac:  _move v1,v0
80030bb0:  subu v1,zero,v1
80030bb4:  mult v1,v0
80030bb8:  mflo v1
80030bbc:  lui v0,0x2aaa
80030bc0:  ori v0,v0,0xaaab
80030bc4:  mult v1,v0
80030bc8:  sra v1,v1,0x1f
80030bcc:  mfhi v0
80030bd0:  sra v0,v0,0x2
80030bd4:  subu v0,v0,v1
80030bd8:  sh v0,0x0(a2)
80030bdc:  andi v0,a3,0x100
80030be0:  bne v0,zero,0x80030bfc
80030be4:  _li v0,0x80
80030be8:  lbu v1,0x11(a1)
80030bec:  li v0,0x80
80030bf0:  subu v0,v0,v1
80030bf4:  jr ra
80030bf8:  _sh v0,0x2(a2)
80030bfc:  sh v0,0x2(a2)
80030c00:  jr ra
80030c04:  _nop
