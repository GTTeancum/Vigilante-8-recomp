# addr: 0x80054e80  name: _padGetActSize
80054e80:  lbu v0,0xe3(a0)
80054e84:  lbu a1,0xe9(a0)
80054e88:  lw a0,0xec(a0)
80054e8c:  addiu v0,v0,0x1
80054e90:  sra v0,v0,0x1
80054e94:  sll v0,v0,0x2
80054e98:  sll v1,a1,0x2
80054e9c:  addu v1,v1,a1
80054ea0:  addiu v1,v1,0x3
80054ea4:  andi v1,v1,0xffc
80054ea8:  addiu v1,v1,0x4
80054eac:  addu v0,v0,v1
80054eb0:  jr ra
80054eb4:  _addu v0,v0,a0
