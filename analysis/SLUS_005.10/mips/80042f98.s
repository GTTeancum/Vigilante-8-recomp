# addr: 0x80042f98  name: FUN_80042f98
80042f98:  addiu sp,sp,-0x28
80042f9c:  sw ra,0x20(sp)
80042fa0:  lw v0,0x8(a1)
80042fa4:  lw v1,0x24(a0)
80042fa8:  nop
80042fac:  subu v0,v0,v1
80042fb0:  bgez v0,0x80042fbc
80042fb4:  _nop
80042fb8:  subu v0,zero,v0
80042fbc:  slt v0,v0,a2
80042fc0:  beq v0,zero,0x80043048
80042fc4:  _nop
80042fc8:  lw v0,0xc(a1)
80042fcc:  lw v1,0x2c(a0)
80042fd0:  nop
80042fd4:  subu v0,v0,v1
80042fd8:  bgez v0,0x80042fe4
80042fdc:  _nop
80042fe0:  subu v0,zero,v0
80042fe4:  slt v0,v0,a2
80042fe8:  beq v0,zero,0x80043048
80042fec:  _nop
80042ff0:  lh v0,0x0(a1)
80042ff4:  nop
80042ff8:  blez v0,0x80043044
80042ffc:  _li v0,-0x1
80043000:  lhu v0,0x2(a1)
80043004:  lw v1,0x4(a1)
80043008:  addiu v0,v0,0x1
8004300c:  sh v0,0x2(a1)
80043010:  sll v0,v0,0x10
80043014:  sra v0,v0,0xe
80043018:  addu v0,v0,v1
8004301c:  lw v1,0x0(v0)
80043020:  nop
80043024:  beq v1,zero,0x80043040
80043028:  _sll v0,v1,0x10
8004302c:  sw v0,0x8(a1)
80043030:  lui v0,0xffff
80043034:  and v0,v1,v0
80043038:  j 0x80043048
8004303c:  _sw v0,0xc(a1)
80043040:  li v0,-0x1
80043044:  sh v0,0x0(a1)
80043048:  lw v0,0x8(a1)
8004304c:  lw v1,0x24(a0)
80043050:  sw zero,0x14(sp)
80043054:  subu v0,v0,v1
80043058:  sw v0,0x10(sp)
8004305c:  lw v0,0xc(a1)
80043060:  lw v1,0x2c(a0)
80043064:  addiu a0,a0,0x10
80043068:  addiu a1,sp,0x10
8004306c:  move a2,a1
80043070:  subu v0,v0,v1
80043074:  jal 0x8004352c
80043078:  _sw v0,0x18(sp)
8004307c:  lw a0,0x10(sp)
80043080:  lw a1,0x18(sp)
80043084:  jal 0x8004ecd4
80043088:  _nop
8004308c:  lw ra,0x20(sp)
80043090:  sll v0,v0,0x14
80043094:  sra v0,v0,0x14
80043098:  jr ra
8004309c:  _addiu sp,sp,0x28
