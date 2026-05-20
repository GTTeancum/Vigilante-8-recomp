# addr: 0x80045e24  name: _spu_FsetRXXa
80045e24:  lui v0,0x8006
80045e28:  lw v0,-0x1208(v0)
80045e2c:  nop
80045e30:  beq v0,zero,0x80045e74
80045e34:  _move a2,a0
80045e38:  lui a0,0x8006
80045e3c:  lw a0,-0x1200(a0)
80045e40:  nop
80045e44:  divu a1,a0
80045e48:  bne a0,zero,0x80045e54
80045e4c:  _nop
80045e50:  break 0x1c00
80045e54:  mfhi v0
80045e58:  beq v0,zero,0x80045e74
80045e5c:  _nop
80045e60:  lui v0,0x8006
80045e64:  lw v0,-0x11fc(v0)
80045e68:  addu a1,a1,a0
80045e6c:  nor v0,zero,v0
80045e70:  and a1,a1,v0
80045e74:  lui v0,0x8006
80045e78:  lw v0,-0x1204(v0)
80045e7c:  nop
80045e80:  srlv a3,a1,v0
80045e84:  li v0,-0x2
80045e88:  beq a2,v0,0x80045ea4
80045e8c:  _move v1,a3
80045e90:  li v0,-0x1
80045e94:  bne a2,v0,0x80045eac
80045e98:  _move v0,a1
80045e9c:  j 0x80045ec0
80045ea0:  _andi v0,v1,0xffff
80045ea4:  j 0x80045ec0
80045ea8:  _move v0,a1
80045eac:  lui a0,0x8006
80045eb0:  lw a0,-0x122c(a0)
80045eb4:  sll v1,a2,0x1
80045eb8:  addu v1,v1,a0
80045ebc:  sh a3,0x0(v1)
