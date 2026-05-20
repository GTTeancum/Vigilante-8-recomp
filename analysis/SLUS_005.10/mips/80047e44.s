# addr: 0x80047e44  name: VSync
80047e44:  lui v0,0x8006
80047e48:  lw v0,-0x1184(v0)
80047e4c:  lui a1,0x8006
80047e50:  lw a1,-0x1180(a1)
80047e54:  addiu sp,sp,-0x28
80047e58:  sw ra,0x20(sp)
80047e5c:  sw s1,0x1c(sp)
80047e60:  sw s0,0x18(sp)
80047e64:  lw s0,0x0(v0)
80047e68:  lw v0,0x0(a1)
80047e6c:  nop
80047e70:  sw v0,0x10(sp)
80047e74:  lw v1,0x10(sp)
80047e78:  lw v0,0x0(a1)
80047e7c:  nop
80047e80:  bne v1,v0,0x80047e68
80047e84:  _nop
80047e88:  lw v0,0x10(sp)
80047e8c:  lui v1,0x8006
80047e90:  lw v1,-0x117c(v1)
80047e94:  nop
80047e98:  subu v0,v0,v1
80047e9c:  bgez a0,0x80047eb4
80047ea0:  _andi s1,v0,0xffff
80047ea4:  lui v0,0x8006
80047ea8:  lw v0,-0x4c(v0)
80047eac:  j 0x80047fa8
80047eb0:  _nop
80047eb4:  li v0,0x1
80047eb8:  beq a0,v0,0x80047fa4
80047ebc:  _nop
80047ec0:  blez a0,0x80047ee0
80047ec4:  _nop
80047ec8:  lui v0,0x8006
80047ecc:  lw v0,-0x1178(v0)
80047ed0:  nop
80047ed4:  addiu v0,v0,-0x1
80047ed8:  j 0x80047ee8
80047edc:  _addu v0,v0,a0
80047ee0:  lui v0,0x8006
80047ee4:  lw v0,-0x1178(v0)
