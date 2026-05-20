# addr: 0x80100c6c  name: FUN_80100c6c
80100c6c:  addiu sp,sp,-0x18
80100c70:  sw s0,0x10(sp)
80100c74:  move s0,a0
80100c78:  beq a1,zero,0x80100c94
80100c7c:  _sw ra,0x14(sp)
80100c80:  li v0,0x2
80100c84:  beq a1,v0,0x80100cf4
80100c88:  _nop
80100c8c:  slti a0,zero,0x390
80100c90:  nop
80100c94:  lw v1,0x80(s0)
80100c98:  nop
80100c9c:  bgez v1,0x80100ca8
80100ca0:  _nop
80100ca4:  addiu v1,v1,0x7f
80100ca8:  lw v0,0x24(s0)
80100cac:  lw a1,0x84(s0)
80100cb0:  sra v1,v1,0x7
80100cb4:  addu v0,v0,v1
80100cb8:  bgez a1,0x80100cc4
80100cbc:  _sw v0,0x24(s0)
80100cc0:  addiu a1,a1,0x7f
80100cc4:  lw v0,0x28(s0)
80100cc8:  lw a0,0x88(s0)
80100ccc:  sra v1,a1,0x7
80100cd0:  addu v0,v0,v1
80100cd4:  bgez a0,0x80100ce0
80100cd8:  _sw v0,0x28(s0)
80100cdc:  addiu a0,a0,0x7f
80100ce0:  lw v0,0x2c(s0)
80100ce4:  sra v1,a0,0x7
80100ce8:  addu v0,v0,v1
80100cec:  slti a0,zero,0x390
80100cf0:  sw v0,0x2c(s0)
80100cf4:  lw v1,0x0(s0)
80100cf8:  nop
80100cfc:  andi v0,v1,0x2
80100d00:  beq v0,zero,0x80100e2c
80100d04:  _lui v0,0xfeff
80100d08:  ori v0,v0,0xfffd
80100d0c:  move a0,s0
80100d10:  and v0,v1,v0
80100d14:  jal 0x8001d708
80100d18:  _sw v0,0x0(s0)
80100d1c:  lh v1,0x14(s0)
80100d20:  nop
80100d24:  sll v0,v1,0x3
80100d28:  addu v0,v0,v1
80100d2c:  sll v0,v0,0x4
80100d30:  subu v0,v0,v1
80100d34:  sll v0,v0,0x4
80100d38:  addu v0,v0,v1
80100d3c:  sll v0,v0,0x2
80100d40:  subu v0,v0,v1
80100d44:  sll v0,v0,0x1
80100d48:  bgez v0,0x80100d54
80100d4c:  _addiu a0,s0,0x80
80100d50:  addiu v0,v0,0x1f
80100d54:  lh v1,0x1a(s0)
80100d58:  sra v0,v0,0x5
80100d5c:  sw v0,0x80(s0)
80100d60:  sll v0,v1,0x3
80100d64:  addu v0,v0,v1
80100d68:  sll v0,v0,0x4
80100d6c:  subu v0,v0,v1
80100d70:  sll v0,v0,0x4
80100d74:  addu v0,v0,v1
80100d78:  sll v0,v0,0x2
80100d7c:  subu v0,v0,v1
80100d80:  sll v0,v0,0x1
80100d84:  bgez v0,0x80100d90
80100d88:  _nop
80100d8c:  addiu v0,v0,0x1f
80100d90:  sra v0,v0,0x5
80100d94:  sw v0,0x4(a0)
80100d98:  lh v1,0x20(s0)
80100d9c:  nop
80100da0:  sll v0,v1,0x3
80100da4:  addu v0,v0,v1
80100da8:  sll v0,v0,0x4
80100dac:  subu v0,v0,v1
80100db0:  sll v0,v0,0x4
80100db4:  addu v0,v0,v1
80100db8:  sll v0,v0,0x2
80100dbc:  subu v0,v0,v1
80100dc0:  sll v0,v0,0x1
80100dc4:  bgez v0,0x80100dd0
80100dc8:  _nop
80100dcc:  addiu v0,v0,0x1f
80100dd0:  sra v0,v0,0x5
80100dd4:  sw v0,0x8(a0)
80100dd8:  lw a0,0xe0(s0)
80100ddc:  nop
80100de0:  beq a0,zero,0x80100df0
80100de4:  _nop
80100de8:  jal 0x80020890
80100dec:  _li a1,0x3c
80100df0:  move a0,s0
80100df4:  jal 0x80020890
80100df8:  _li a1,0x1e
80100dfc:  jal 0x8004410c
80100e00:  _nop
80100e04:  lui v1,0x8007
80100e08:  lw v1,0x37e8(v1)
80100e0c:  nop
80100e10:  lw a1,0x8(v1)
80100e14:  move a0,v0
80100e18:  li a2,0x4
80100e1c:  jal 0x800447e8
80100e20:  _addiu a3,s0,0x24
80100e24:  slti a0,zero,0x390
80100e28:  nop
80100e2c:  li v0,-0x21
80100e30:  and v0,v1,v0
80100e34:  sw v0,0x0(s0)
80100e38:  jal 0x80031294
80100e3c:  _move a0,s0
80100e40:  lw ra,0x14(sp)
80100e44:  lw s0,0x10(sp)
80100e48:  clear v0
80100e4c:  jr ra
80100e50:  _addiu sp,sp,0x18
