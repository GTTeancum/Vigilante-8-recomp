# addr: 0x8010efa4  name: FUN_8010efa4
8010efa4:  addiu sp,sp,-0x20
8010efa8:  sw s0,0x10(sp)
8010efac:  move s0,a0
8010efb0:  sw ra,0x18(sp)
8010efb4:  sw s1,0x14(sp)
8010efb8:  lw v1,0x0(s0)
8010efbc:  nop
8010efc0:  beq v1,zero,0x8010efe8
8010efc4:  _li v0,0xa
8010efc8:  bltz v1,0x8010f160
8010efcc:  _li s1,0xa
8010efd0:  beq v1,s1,0x8010effc
8010efd4:  _li v0,0xb
8010efd8:  beq v1,v0,0x8010f028
8010efdc:  _clear v0
8010efe0:  slti a0,zero,0x3c59
8010efe4:  nop
8010efe8:  lui at,0x8011
8010efec:  sw zero,0x3424(at)
8010eff0:  lui at,0x8011
8010eff4:  sw zero,0x3420(at)
8010eff8:  sw v0,0x0(s0)
8010effc:  sltiu a0,zero,0x4420
8010f000:  nop
8010f004:  lui a0,0x8011
8010f008:  lw a0,0x3444(a0)
8010f00c:  sltiu a0,zero,0x4214
8010f010:  nop
8010f014:  lw v0,0x0(s0)
8010f018:  nop
8010f01c:  addiu v0,v0,0x1
8010f020:  slti a0,zero,0x3c58
8010f024:  sw v0,0x0(s0)
8010f028:  sltiu a0,zero,0x44ce
8010f02c:  nop
8010f030:  beq v0,zero,0x8010f164
8010f034:  _clear v0
8010f038:  sltiu a0,zero,0x4462
8010f03c:  nop
8010f040:  move v1,v0
8010f044:  lui at,0x8011
8010f048:  sw v0,0x3424(at)
8010f04c:  slti v0,v1,0x3
8010f050:  beq v0,zero,0x8010f070
8010f054:  _nop
8010f058:  bgtz v1,0x8010f0e8
8010f05c:  _nop
8010f060:  beq v1,zero,0x8010f098
8010f064:  _li v0,0x1
8010f068:  slti a0,zero,0x3c45
8010f06c:  nop
8010f070:  li v0,0x4
8010f074:  bne v1,v0,0x8010f114
8010f078:  _nop
8010f07c:  sltiu a0,zero,0x41c5
8010f080:  li a0,0x4
8010f084:  lui v1,0x8011
8010f088:  addiu v1,v1,0x3438
8010f08c:  sw v0,0x4(v1)
8010f090:  slti a0,zero,0x3c59
8010f094:  li v0,0x1
8010f098:  lui s0,0x8011
8010f09c:  addiu s0,s0,0x3444
8010f0a0:  lw a0,0x0(s0)
8010f0a4:  lui v1,0x8011
8010f0a8:  lw v1,0x3434(v1)
8010f0ac:  sllv v0,v0,a0
8010f0b0:  and v1,v1,v0
8010f0b4:  bne v1,zero,0x8010f0c4
8010f0b8:  _li v0,0x4
8010f0bc:  lui at,0x8011
8010f0c0:  sw v0,0x3424(at)
8010f0c4:  lui a0,0x8011
8010f0c8:  lw a0,0x3424(a0)
8010f0cc:  sltiu a0,zero,0x41c5
8010f0d0:  nop
8010f0d4:  move a0,v0
8010f0d8:  li v0,0x1
8010f0dc:  addiu v1,s0,-0xc
8010f0e0:  slti a0,zero,0x3c59
8010f0e4:  sw a0,0x4(v1)
8010f0e8:  lui v0,0x8011
8010f0ec:  lw v0,0x3420(v0)
8010f0f0:  nop
8010f0f4:  addiu v0,v0,0x1
8010f0f8:  lui at,0x8011
8010f0fc:  sw v0,0x3420(at)
8010f100:  slti v0,v0,0x5
8010f104:  beq v0,zero,0x8010f114
8010f108:  _nop
8010f10c:  slti a0,zero,0x3c58
8010f110:  sw s1,0x0(s0)
8010f114:  lui s0,0x8011
8010f118:  addiu s0,s0,0x3444
8010f11c:  li v0,0x1
8010f120:  lw v1,0x0(s0)
8010f124:  lui a0,0x8011
8010f128:  lw a0,0x3424(a0)
8010f12c:  sllv v0,v0,v1
8010f130:  lui v1,0x8011
8010f134:  lw v1,0x3434(v1)
8010f138:  nor v0,zero,v0
8010f13c:  and v1,v1,v0
8010f140:  lui at,0x8011
8010f144:  sw v1,0x3434(at)
8010f148:  sltiu a0,zero,0x41c5
8010f14c:  addiu s0,s0,-0xc
8010f150:  move v1,v0
8010f154:  li v0,0x1
8010f158:  slti a0,zero,0x3c59
8010f15c:  sw v1,0x4(s0)
8010f160:  clear v0
8010f164:  lw ra,0x18(sp)
8010f168:  lw s1,0x14(sp)
8010f16c:  lw s0,0x10(sp)
8010f170:  jr ra
8010f174:  _addiu sp,sp,0x20
