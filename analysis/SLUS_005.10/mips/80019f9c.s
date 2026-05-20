# addr: 0x80019f9c  name: FUN_80019f9c
80019f9c:  addiu sp,sp,-0x48
80019fa0:  sw s0,0x38(sp)
80019fa4:  move s0,a0
80019fa8:  sw s1,0x3c(sp)
80019fac:  move s1,a1
80019fb0:  lui v1,0x5555
80019fb4:  ori v1,v1,0x5555
80019fb8:  li v0,0x6
80019fbc:  sb v0,0x1b(sp)
80019fc0:  li v0,0x4c
80019fc4:  sb v0,0x1f(sp)
80019fc8:  srl v0,s1,0x8
80019fcc:  sb v0,0x1d(sp)
80019fd0:  srl v0,s1,0x10
80019fd4:  sw ra,0x40(sp)
80019fd8:  sw v1,0x30(sp)
80019fdc:  sb s1,0x1c(sp)
80019fe0:  sb v0,0x1e(sp)
80019fe4:  lhu v0,0x0(s0)
80019fe8:  nop
80019fec:  sh v0,0x20(sp)
80019ff0:  lhu v0,0x2(s0)
80019ff4:  nop
80019ff8:  sh v0,0x22(sp)
80019ffc:  lhu v0,0x0(s0)
8001a000:  lhu v1,0x4(s0)
8001a004:  nop
8001a008:  addu v0,v0,v1
8001a00c:  addiu v0,v0,-0x1
8001a010:  sh v0,0x24(sp)
8001a014:  lhu v0,0x2(s0)
8001a018:  nop
8001a01c:  sh v0,0x26(sp)
8001a020:  lhu v0,0x0(s0)
8001a024:  lhu v1,0x4(s0)
8001a028:  nop
8001a02c:  addu v0,v0,v1
8001a030:  addiu v0,v0,-0x1
8001a034:  sh v0,0x28(sp)
8001a038:  lhu v0,0x2(s0)
8001a03c:  lhu v1,0x6(s0)
8001a040:  nop
8001a044:  addu v0,v0,v1
8001a048:  addiu v0,v0,-0x1
8001a04c:  sh v0,0x2a(sp)
8001a050:  lhu v0,0x0(s0)
8001a054:  nop
8001a058:  sh v0,0x2c(sp)
8001a05c:  lhu v0,0x2(s0)
8001a060:  lhu v1,0x6(s0)
8001a064:  addiu a0,sp,0x18
8001a068:  addu v0,v0,v1
8001a06c:  addiu v0,v0,-0x1
8001a070:  jal 0x8004fb18
8001a074:  _sh v0,0x2e(sp)
8001a078:  lh a0,0x0(s0)
8001a07c:  lh a1,0x2(s0)
8001a080:  lh a3,0x6(s0)
8001a084:  sw s1,0x10(sp)
8001a088:  addu a3,a1,a3
8001a08c:  move a2,a0
8001a090:  jal 0x80019f44
8001a094:  _addiu a3,a3,-0x1
8001a098:  lw ra,0x40(sp)
8001a09c:  lw s1,0x3c(sp)
8001a0a0:  lw s0,0x38(sp)
8001a0a4:  jr ra
8001a0a8:  _addiu sp,sp,0x48
