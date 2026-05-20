# addr: 0x80044c44  name: FUN_80044c44
80044c44:  beq a2,zero,0x80044d8c
80044c48:  _move v0,a0
80044c4c:  andi t0,a1,0x3
80044c50:  beq t0,zero,0x80044c78
80044c54:  _andi t0,a0,0x3
80044c58:  lbu t0,0x0(a1)
80044c5c:  addi a1,a1,0x1
80044c60:  sb t0,0x0(a0)
80044c64:  addi a2,a2,-0x1
80044c68:  bne a2,zero,0x80044c4c
80044c6c:  _addi a0,a0,0x1
80044c70:  jr ra
80044c74:  _nop
80044c78:  bne t0,zero,0x80044cf8
80044c7c:  _addiu a2,a2,-0x10
80044c80:  bltz a2,0x80044cb8
80044c84:  _nop
80044c88:  lw t0,0x0(a1)
80044c8c:  lw t1,0x4(a1)
80044c90:  lw t2,0x8(a1)
80044c94:  lw t3,0xc(a1)
80044c98:  sw t0,0x0(a0)
80044c9c:  sw t1,0x4(a0)
80044ca0:  sw t2,0x8(a0)
80044ca4:  sw t3,0xc(a0)
80044ca8:  addi a1,a1,0x10
80044cac:  addi a2,a2,-0x10
80044cb0:  bgez a2,0x80044c88
80044cb4:  _addi a0,a0,0x10
80044cb8:  addiu a2,a2,0xc
80044cbc:  bltz a2,0x80044cdc
80044cc0:  _nop
80044cc4:  lw t0,0x0(a1)
80044cc8:  addi a1,a1,0x4
80044ccc:  sw t0,0x0(a0)
80044cd0:  addi a2,a2,-0x4
80044cd4:  bgez a2,0x80044cc4
80044cd8:  _addi a0,a0,0x4
80044cdc:  addiu a2,a2,0x3
80044ce0:  bltz a2,0x80044d8c
80044ce4:  _add a1,a1,a2
80044ce8:  lwl t0,0x0(a1)
80044cec:  add a0,a0,a2
80044cf0:  jr ra
80044cf4:  _swl t0,0x0(a0)
80044cf8:  bltz a2,0x80044d40
80044cfc:  _nop
80044d00:  lw t0,0x0(a1)
80044d04:  lw t1,0x4(a1)
80044d08:  lw t2,0x8(a1)
80044d0c:  lw t3,0xc(a1)
80044d10:  swr t0,0x0(a0)
80044d14:  swl t0,0x3(a0)
80044d18:  swr t1,0x4(a0)
80044d1c:  swl t1,0x7(a0)
80044d20:  swr t2,0x8(a0)
80044d24:  swl t2,0xb(a0)
80044d28:  swr t3,0xc(a0)
80044d2c:  swl t3,0xf(a0)
80044d30:  addi a1,a1,0x10
80044d34:  addi a2,a2,-0x10
80044d38:  bgez a2,0x80044d00
80044d3c:  _addi a0,a0,0x10
80044d40:  addiu a2,a2,0xc
80044d44:  bltz a2,0x80044d68
80044d48:  _nop
80044d4c:  lw t0,0x0(a1)
80044d50:  addi a1,a1,0x4
80044d54:  swr t0,0x0(a0)
80044d58:  swl t0,0x3(a0)
80044d5c:  addi a2,a2,-0x4
80044d60:  bgez a2,0x80044d4c
80044d64:  _addi a0,a0,0x4
80044d68:  addiu a2,a2,0x4
80044d6c:  beq a2,zero,0x80044d8c
80044d70:  _nop
80044d74:  lb t0,0x0(a1)
80044d78:  addi a1,a1,0x1
80044d7c:  sb t0,0x0(a0)
80044d80:  addi a2,a2,-0x1
80044d84:  bne a2,zero,0x80044d74
80044d88:  _addi a0,a0,0x1
80044d8c:  jr ra
80044d90:  _nop
