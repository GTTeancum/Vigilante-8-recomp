# addr: 0x80046d84  name: SpuSetVoiceAttr
80046d84:  addiu sp,sp,-0x38
80046d88:  sw s0,0x18(sp)
80046d8c:  move s0,a0
80046d90:  sw s4,0x28(sp)
80046d94:  clear s4
80046d98:  sw ra,0x30(sp)
80046d9c:  sw s5,0x2c(sp)
80046da0:  sw s3,0x24(sp)
80046da4:  sw s2,0x20(sp)
80046da8:  sw s1,0x1c(sp)
80046dac:  lw s1,0x4(s0)
80046db0:  lui s5,0x8006
80046db4:  addiu s5,s5,-0x1270
80046db8:  sltiu s2,s1,0x1
80046dbc:  li v0,0x1
80046dc0:  lw v1,0x0(s0)
80046dc4:  sllv v0,v0,s4
80046dc8:  and v1,v1,v0
80046dcc:  beq v1,zero,0x800472f8
80046dd0:  _nop
80046dd4:  bne s2,zero,0x80046de8
80046dd8:  _sll s3,s4,0x3
80046ddc:  andi v0,s1,0x10
80046de0:  beq v0,zero,0x80046e00
80046de4:  _nop
80046de8:  sll v0,s4,0x4
80046dec:  lui v1,0x8006
80046df0:  lw v1,-0x122c(v1)
80046df4:  lhu a0,0x14(s0)
80046df8:  addu v0,v0,v1
80046dfc:  sh a0,0x4(v0)
80046e00:  bne s2,zero,0x80046e10
80046e04:  _andi v0,s1,0x40
80046e08:  beq v0,zero,0x80046e1c
80046e0c:  _nop
80046e10:  lhu v0,0x18(s0)
80046e14:  nop
80046e18:  sh v0,0x0(s5)
80046e1c:  bne s2,zero,0x80046e2c
80046e20:  _andi v0,s1,0x20
80046e24:  beq v0,zero,0x80046e5c
80046e28:  _nop
80046e2c:  lhu a1,0x0(s5)
80046e30:  lhu a3,0x16(s0)
80046e34:  srl a0,a1,0x8
80046e38:  andi a1,a1,0xff
80046e3c:  srl a2,a3,0x8
80046e40:  jal 0x80047410
80046e44:  _andi a3,a3,0xff
80046e48:  lui a0,0x8006
80046e4c:  lw a0,-0x122c(a0)
80046e50:  sll v1,s3,0x1
80046e54:  addu v1,v1,a0
80046e58:  sh v0,0x4(v1)
80046e5c:  bne s2,zero,0x80046e6c
80046e60:  _andi v0,s1,0x1
80046e64:  beq v0,zero,0x80046f3c
80046e68:  _nop
80046e6c:  lhu v0,0x8(s0)
80046e70:  clear a0
80046e74:  bne s2,zero,0x80046e88
80046e78:  _andi a1,v0,0x7fff
80046e7c:  andi v0,s1,0x4
80046e80:  beq v0,zero,0x80046ef4
80046e84:  _nop
80046e88:  lhu v0,0xc(s0)
80046e8c:  nop
80046e90:  addiu v0,v0,-0x1
80046e94:  sll v0,v0,0x10
80046e98:  sra v1,v0,0x10
80046e9c:  sltiu v0,v1,0x7
80046ea0:  beq v0,zero,0x80046ef4
80046ea4:  _sll v0,v1,0x2
80046ea8:  lui at,0x8001
80046eac:  addu at,at,v0
80046eb0:  lw v0,0xc24(at)
80046eb4:  nop
80046eb8:  jr v0
80046ebc:  _nop
