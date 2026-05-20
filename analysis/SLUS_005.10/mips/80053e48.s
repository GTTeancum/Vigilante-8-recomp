# addr: 0x80053e48  name: PadInfoAct
80053e48:  lui v0,0x8006
80053e4c:  lw v0,0x5270(v0)
80053e50:  addiu sp,sp,-0x20
80053e54:  sw s0,0x10(sp)
80053e58:  move s0,a1
80053e5c:  sw s1,0x14(sp)
80053e60:  sw ra,0x18(sp)
80053e64:  jalr v0
80053e68:  _move s1,a2
80053e6c:  bgez s0,0x80053e80
80053e70:  _move v1,v0
80053e74:  lbu v0,0xe9(v1)
80053e78:  j 0x80053f08
80053e7c:  _nop
80053e80:  lbu v0,0xe9(v1)
80053e84:  nop
80053e88:  slt v0,s0,v0
80053e8c:  beq v0,zero,0x80053f04
80053e90:  _sll v0,s0,0x2
80053e94:  lw v1,0x4(v1)
80053e98:  addu v0,v0,s0
80053e9c:  addiu a2,s1,-0x1
80053ea0:  addu v1,v1,v0
80053ea4:  sltiu v0,a2,0x5
80053ea8:  beq v0,zero,0x80053f04
80053eac:  _sll v0,a2,0x2
80053eb0:  lui at,0x8001
80053eb4:  addu at,at,v0
80053eb8:  lw v0,0x1694(at)
80053ebc:  nop
80053ec0:  jr v0
80053ec4:  _nop
80053f04:  clear v0
