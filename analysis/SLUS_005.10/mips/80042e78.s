# addr: 0x80042e78  name: FUN_80042e78
80042e78:  addiu sp,sp,-0x20
80042e7c:  sw s0,0x10(sp)
80042e80:  move s0,a0
80042e84:  sw ra,0x18(sp)
80042e88:  sw s1,0x14(sp)
80042e8c:  lw a0,0x4(s0)
80042e90:  nop
80042e94:  beq a0,zero,0x80042ea4
80042e98:  _move s1,a1
80042e9c:  jal 0x80045088
80042ea0:  _nop
80042ea4:  sltu v0,zero,s1
80042ea8:  sw s1,0x4(s0)
80042eac:  sh zero,0x2(s0)
80042eb0:  beq v0,zero,0x80042ed8
80042eb4:  _sh v0,0x0(s0)
80042eb8:  lh v0,0x0(s1)
80042ebc:  nop
80042ec0:  sll v0,v0,0x10
80042ec4:  sw v0,0x8(s0)
80042ec8:  lh v0,0x2(s1)
80042ecc:  nop
80042ed0:  sll v0,v0,0x10
80042ed4:  sw v0,0xc(s0)
80042ed8:  lh v0,0x0(s0)
80042edc:  lw ra,0x18(sp)
80042ee0:  lw s1,0x14(sp)
80042ee4:  lw s0,0x10(sp)
80042ee8:  jr ra
80042eec:  _addiu sp,sp,0x20
