# addr: 0x800469ec  name: S_SCA_OBJ_158
800469ec:  beq a1,zero,0x80046a18
800469f0:  _andi v0,t0,0x7fff
800469f4:  lh a2,0x6(a0)
800469f8:  lhu v1,0x6(a0)
800469fc:  slti v0,a2,0x80
80046a00:  beq v0,zero,0x80046a14
80046a04:  _li t0,0x7f
80046a08:  bltz a2,0x80046a14
80046a0c:  _clear t0
80046a10:  move t0,v1
80046a14:  andi v0,t0,0x7fff
80046a18:  lui v1,0x8006
80046a1c:  lw v1,-0x122c(v1)
80046a20:  or v0,v0,a1
80046a24:  sh v0,0x182(v1)
80046a28:  bne t2,zero,0x80046a38
80046a2c:  _andi v0,t1,0x40
80046a30:  beq v0,zero,0x80046a4c
80046a34:  _nop
80046a38:  lui v1,0x8006
80046a3c:  lw v1,-0x122c(v1)
80046a40:  lhu v0,0x10(a0)
80046a44:  nop
80046a48:  sh v0,0x1b0(v1)
80046a4c:  bne t2,zero,0x80046a5c
80046a50:  _andi v0,t1,0x80
80046a54:  beq v0,zero,0x80046a70
80046a58:  _nop
80046a5c:  lui v1,0x8006
80046a60:  lw v1,-0x122c(v1)
80046a64:  lhu v0,0x12(a0)
80046a68:  nop
80046a6c:  sh v0,0x1b2(v1)
80046a70:  bne t2,zero,0x80046a80
80046a74:  _andi v0,t1,0x400
80046a78:  beq v0,zero,0x80046a94
80046a7c:  _nop
80046a80:  lui v1,0x8006
80046a84:  lw v1,-0x122c(v1)
80046a88:  lhu v0,0x1c(a0)
80046a8c:  nop
80046a90:  sh v0,0x1b4(v1)
80046a94:  bne t2,zero,0x80046aa4
80046a98:  _andi v0,t1,0x800
80046a9c:  beq v0,zero,0x80046ab8
80046aa0:  _nop
80046aa4:  lui v1,0x8006
80046aa8:  lw v1,-0x122c(v1)
80046aac:  lhu v0,0x1e(a0)
80046ab0:  nop
80046ab4:  sh v0,0x1b6(v1)
80046ab8:  bne t2,zero,0x80046ac8
80046abc:  _andi v0,t1,0x100
80046ac0:  beq v0,zero,0x80046b0c
80046ac4:  _nop
80046ac8:  lw v0,0x14(a0)
80046acc:  nop
80046ad0:  bne v0,zero,0x80046af0
80046ad4:  _nop
80046ad8:  lui v1,0x8006
80046adc:  lw v1,-0x122c(v1)
80046ae0:  nop
80046ae4:  lhu v0,0x1aa(v1)
80046ae8:  j 0x80046b08
80046aec:  _andi v0,v0,0xfffb
80046af0:  lui v1,0x8006
80046af4:  lw v1,-0x122c(v1)
80046af8:  nop
80046afc:  lhu v0,0x1aa(v1)
80046b00:  nop
80046b04:  ori v0,v0,0x4
