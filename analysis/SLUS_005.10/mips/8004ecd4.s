# addr: 0x8004ecd4  name: ratan2
8004ecd4:  clear a2
8004ecd8:  bgez a1,0x8004ece8
8004ecdc:  _clear a3
8004ece0:  li a2,0x1
8004ece4:  subu a1,zero,a1
8004ece8:  bgez a0,0x8004ecf8
8004ecec:  _nop
8004ecf0:  li a3,0x1
8004ecf4:  subu a0,zero,a0
8004ecf8:  bne a1,zero,0x8004ed0c
8004ecfc:  _slt v0,a0,a1
8004ed00:  beq a0,zero,0x8004ee40
8004ed04:  _clear v0
8004ed08:  slt v0,a0,a1
8004ed0c:  beq v0,zero,0x8004ed9c
8004ed10:  _lui v0,0x7fe0
8004ed14:  and v0,a0,v0
8004ed18:  beq v0,zero,0x8004ed54
8004ed1c:  _sra v0,a1,0xa
8004ed20:  div a0,v0
8004ed24:  bne v0,zero,0x8004ed30
8004ed28:  _nop
8004ed2c:  break 0x1c00
8004ed30:  li at,-0x1
8004ed34:  bne v0,at,0x8004ed48
8004ed38:  _lui at,0x8000
8004ed3c:  bne a0,at,0x8004ed48
8004ed40:  _nop
8004ed44:  break 0x1800
8004ed48:  mflo a0
8004ed4c:  j 0x8004ed88
8004ed50:  _sll v0,a0,0x1
8004ed54:  sll v0,a0,0xa
8004ed58:  div v0,a1
8004ed5c:  bne a1,zero,0x8004ed68
8004ed60:  _nop
8004ed64:  break 0x1c00
8004ed68:  li at,-0x1
8004ed6c:  bne a1,at,0x8004ed80
8004ed70:  _lui at,0x8000
8004ed74:  bne v0,at,0x8004ed80
8004ed78:  _nop
8004ed7c:  break 0x1800
8004ed80:  mflo a0
8004ed84:  sll v0,a0,0x1
