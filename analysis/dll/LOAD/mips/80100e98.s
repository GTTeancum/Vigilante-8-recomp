# addr: 0x80100e98  name: FUN_80100e98
80100e98:  addiu sp,sp,-0x28
80100e9c:  sw s1,0x1c(sp)
80100ea0:  clear s1
80100ea4:  sw ra,0x24(sp)
80100ea8:  sw s2,0x20(sp)
80100eac:  sw s0,0x18(sp)
80100eb0:  beq a0,zero,0x80100f90
80100eb4:  _sw a0,0x28(sp)
80100eb8:  lui s2,0x4c47
80100ebc:  ori s2,s2,0x4854
80100ec0:  addiu a0,sp,0x10
80100ec4:  jal 0x800225d4
80100ec8:  _addiu a1,sp,0x28
80100ecc:  lw a0,0x10(sp)
80100ed0:  move s0,v0
80100ed4:  srl v1,a0,0x18
80100ed8:  srl v0,a0,0x8
80100edc:  andi v0,v0,0xff00
80100ee0:  or v1,v1,v0
80100ee4:  andi v0,a0,0xff00
80100ee8:  sll v0,v0,0x8
80100eec:  or v1,v1,v0
80100ef0:  sll a0,a0,0x18
80100ef4:  or v1,v1,a0
80100ef8:  beq v1,s2,0x80100f64
80100efc:  _sltu v0,s2,v1
80100f00:  bne v0,zero,0x80100f20
80100f04:  _lui v0,0x5354
80100f08:  lui v0,0x4845
80100f0c:  ori v0,v0,0x4144
80100f10:  beq v1,v0,0x80100f34
80100f14:  _nop
80100f18:  slti a0,zero,0x3de
80100f1c:  nop
80100f20:  ori v0,v0,0x524e
80100f24:  beq v1,v0,0x80100f48
80100f28:  _nop
80100f2c:  slti a0,zero,0x3de
80100f30:  nop
80100f34:  lw a1,0x14(sp)
80100f38:  sltiu a0,zero,0x1bc
80100f3c:  move a0,s0
80100f40:  slti a0,zero,0x3de
80100f44:  move s1,v0
80100f48:  beq s1,zero,0x80100f78
80100f4c:  _move a0,s1
80100f50:  lw a2,0x14(sp)
80100f54:  sltiu a0,zero,0x347
80100f58:  move a1,s0
80100f5c:  slti a0,zero,0x3de
80100f60:  nop
80100f64:  beq s1,zero,0x80100f78
80100f68:  _move a0,s1
80100f6c:  lw a2,0x14(sp)
80100f70:  sltiu a0,zero,0x388
80100f74:  move a1,s0
80100f78:  jal 0x80045088
80100f7c:  _move a0,s0
80100f80:  lw v0,0x28(sp)
80100f84:  nop
80100f88:  bne v0,zero,0x80100ec0
80100f8c:  _nop
80100f90:  lw ra,0x24(sp)
80100f94:  lw s2,0x20(sp)
80100f98:  lw s1,0x1c(sp)
80100f9c:  lw s0,0x18(sp)
80100fa0:  jr ra
80100fa4:  _addiu sp,sp,0x28
