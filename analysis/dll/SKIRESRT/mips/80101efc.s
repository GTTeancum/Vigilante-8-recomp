# addr: 0x80101efc  name: FUN_80101efc
80101efc:  addiu sp,sp,-0x20
80101f00:  sw s1,0x14(sp)
80101f04:  move s1,a0
80101f08:  li v0,0x1
80101f0c:  sw ra,0x1c(sp)
80101f10:  sw s2,0x18(sp)
80101f14:  beq a1,v0,0x80102068
80101f18:  _sw s0,0x10(sp)
80101f1c:  li v0,0x9
80101f20:  bne a1,v0,0x80102078
80101f24:  _nop
80101f28:  lh v0,0x6(s1)
80101f2c:  nop
80101f30:  bne a2,v0,0x80102078
80101f34:  _lui s2,0x8006
80101f38:  lw v0,0x59fc(s2)
80101f3c:  nop
80101f40:  lw v0,0x94(v0)
80101f44:  nop
80101f48:  slti v0,v0,0x18
80101f4c:  beq v0,zero,0x80102078
80101f50:  _li a2,0x98
80101f54:  lw a0,0x58(s1)
80101f58:  lhu a1,0xa(s1)
80101f5c:  jal 0x8001ac44
80101f60:  _clear a3
80101f64:  move s0,v0
80101f68:  jal 0x8001dc1c
80101f6c:  _move a0,s0
80101f70:  lw v1,0x54(s0)
80101f74:  nop
80101f78:  sll v0,v1,0x3
80101f7c:  addu v0,v0,v1
80101f80:  sll v0,v0,0x2
80101f84:  addu v0,v0,v1
80101f88:  sll v0,v0,0x4
80101f8c:  subu v0,v0,v1
80101f90:  sll v0,v0,0x2
80101f94:  bgez v0,0x80101fa0
80101f98:  _nop
80101f9c:  addiu v0,v0,0xfff
80101fa0:  sra v0,v0,0xc
80101fa4:  sw v0,0x54(s0)
80101fa8:  li v0,0x3e8
80101fac:  jal 0x80017160
80101fb0:  _sh v0,0x6(s0)
80101fb4:  sb v0,0x9(s0)
80101fb8:  lw v0,0x0(s0)
80101fbc:  lhu v1,0xe(s1)
80101fc0:  ori v0,v0,0x180
80101fc4:  sh v1,0xc(s0)
80101fc8:  sw v0,0x0(s0)
80101fcc:  lw t0,0x48(s1)
80101fd0:  lw t1,0x4c(s1)
80101fd4:  lw t2,0x50(s1)
80101fd8:  sw t0,0x48(s0)
80101fdc:  sw t1,0x4c(s0)
80101fe0:  sw t2,0x50(s0)
80101fe4:  lw v1,0x54(s0)
80101fe8:  lui v0,0x8010
80101fec:  addiu v0,v0,0x1a94
80101ff0:  sw v0,0x64(s0)
80101ff4:  li v0,-0xbeb
80101ff8:  sw v0,0x84(s0)
80101ffc:  sll v0,v1,0x4
80102000:  addu v0,v0,v1
80102004:  sll v0,v0,0x2
80102008:  subu v0,v0,v1
8010200c:  sll v0,v0,0x6
80102010:  addu v0,v0,v1
80102014:  sll v1,v0,0x2
80102018:  subu v1,v1,v0
8010201c:  bgez v1,0x80102028
80102020:  _nop
80102024:  addiu v1,v1,0xfff
80102028:  sra v1,v1,0xc
8010202c:  lui v0,0x100
80102030:  div v0,v1
80102034:  mflo v0
80102038:  move a0,s0
8010203c:  jal 0x8001d708
80102040:  _sh v0,0x94(s0)
80102044:  jal 0x800202f4
80102048:  _move a0,s0
8010204c:  lw v1,0x59fc(s2)
80102050:  nop
80102054:  lw v0,0x94(v1)
80102058:  nop
8010205c:  addiu v0,v0,0x1
80102060:  slti a0,zero,0x81e
80102064:  sw v0,0x94(v1)
80102068:  lw v0,0x0(s1)
8010206c:  nop
80102070:  ori v0,v0,0x22
80102074:  sw v0,0x0(s1)
80102078:  lw ra,0x1c(sp)
8010207c:  lw s2,0x18(sp)
80102080:  lw s1,0x14(sp)
80102084:  lw s0,0x10(sp)
80102088:  clear v0
8010208c:  jr ra
80102090:  _addiu sp,sp,0x20
