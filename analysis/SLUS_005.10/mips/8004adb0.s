# addr: 0x8004adb0  name: CDREAD_OBJ_9C
8004adb0:  jal 0x80049534
8004adb4:  _addiu a0,sp,0x10
8004adb8:  lui s0,0x8006
8004adbc:  addiu s0,s0,0x3b8
8004adc0:  lw v1,0x0(s0)
8004adc4:  nop
8004adc8:  beq v0,v1,0x8004ade8
8004adcc:  _nop
8004add0:  lui a0,0x8001
8004add4:  jal 0x80053884
8004add8:  _addiu a0,a0,0x10d4
8004addc:  addiu v1,s0,-0x20
8004ade0:  li v0,-0x1
8004ade4:  sw v0,0x14(v1)
8004ade8:  lui s0,0x8006
8004adec:  addiu s0,s0,0x3c8
8004adf0:  lw v0,0x0(s0)
8004adf4:  nop
8004adf8:  andi v0,v0,0x1
8004adfc:  beq v0,zero,0x8004ae1c
8004ae00:  _nop
8004ae04:  lw a0,-0x28(s0)
8004ae08:  lw a1,-0x20(s0)
8004ae0c:  jal 0x800493cc
8004ae10:  _nop
8004ae14:  j 0x8004ae6c
8004ae18:  _nop
8004ae1c:  lw a0,-0x28(s0)
8004ae20:  lw a1,-0x20(s0)
8004ae24:  jal 0x800493ac
8004ae28:  _nop
8004ae2c:  addiu a0,s0,-0x30
8004ae30:  lw v0,-0x20(s0)
8004ae34:  lw v1,-0x28(s0)
8004ae38:  sll v0,v0,0x2
8004ae3c:  addu v1,v1,v0
8004ae40:  sw v1,0x8(a0)
8004ae44:  lw v0,0x14(a0)
8004ae48:  nop
8004ae4c:  addiu v0,v0,-0x1
8004ae50:  sw v0,0x14(a0)
8004ae54:  lw v0,0x20(a0)
8004ae58:  nop
8004ae5c:  addiu v0,v0,0x1
8004ae60:  j 0x8004ae6c
8004ae64:  _sw v0,0x20(a0)
