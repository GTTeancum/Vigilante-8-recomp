# addr: 0x80011c58  name: FUN_80011c58
80011c58:  lbu v0,0x0(a0)
80011c5c:  nop
80011c60:  sb v0,0x15(gp)
80011c64:  lbu v1,0x1(a0)
80011c68:  clear a1
80011c6c:  lui v0,0x8006
80011c70:  addiu a2,v0,0x5674
80011c74:  sb v1,0x5f4(gp)
80011c78:  addu v0,a0,a1
80011c7c:  lbu v1,0x2(v0)
80011c80:  addu v0,a1,a2
80011c84:  addiu a1,a1,0x1
80011c88:  sb v1,0x0(v0)
80011c8c:  slti v0,a1,0x6
80011c90:  bne v0,zero,0x80011c7c
80011c94:  _addu v0,a0,a1
80011c98:  clear a1
80011c9c:  lui v0,0x8006
80011ca0:  addiu a2,v0,0x567c
80011ca4:  addu v0,a0,a1
80011ca8:  lbu v1,0x8(v0)
80011cac:  addu v0,a1,a2
80011cb0:  addiu a1,a1,0x1
80011cb4:  sb v1,0x0(v0)
80011cb8:  slti v0,a1,0x4
80011cbc:  bne v0,zero,0x80011ca8
80011cc0:  _addu v0,a0,a1
80011cc4:  jr ra
80011cc8:  _nop
