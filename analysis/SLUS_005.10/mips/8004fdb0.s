# addr: 0x8004fdb0  name: PutDispEnv
8004fdb0:  addiu sp,sp,-0x20
8004fdb4:  sw s1,0x14(sp)
8004fdb8:  move s1,a0
8004fdbc:  sw s2,0x18(sp)
8004fdc0:  lui s2,0x8006
8004fdc4:  addiu s2,s2,0x5026
8004fdc8:  sw ra,0x1c(sp)
8004fdcc:  sw s0,0x10(sp)
8004fdd0:  lbu v0,0x0(s2)
8004fdd4:  nop
8004fdd8:  sltiu v0,v0,0x2
8004fddc:  bne v0,zero,0x8004fe00
8004fde0:  _lui s0,0x800
8004fde4:  lui a0,0x8001
8004fde8:  addiu a0,a0,0x1328
8004fdec:  lui v0,0x8006
8004fdf0:  lw v0,0x5020(v0)
8004fdf4:  nop
8004fdf8:  jalr v0
8004fdfc:  _move a1,s1
8004fe00:  lui v0,0x500
8004fe04:  lhu v1,0x2(s1)
8004fe08:  lhu a0,0x0(s1)
8004fe0c:  lui a1,0x8006
8004fe10:  lw a1,0x501c(a1)
8004fe14:  andi v1,v1,0x3ff
8004fe18:  sll v1,v1,0xa
8004fe1c:  andi a0,a0,0x3ff
8004fe20:  or a0,a0,v0
8004fe24:  lw v0,0x10(a1)
8004fe28:  nop
8004fe2c:  jalr v0
8004fe30:  _or a0,v1,a0
8004fe34:  lw v1,0x7a(s2)
8004fe38:  lw v0,0x10(s1)
8004fe3c:  nop
8004fe40:  bne v1,v0,0x8004fea8
8004fe44:  _addiu a0,s2,0x6a
8004fe48:  lhu v0,0x6a(s2)
8004fe4c:  lh v1,0x0(s1)
8004fe50:  sll v0,v0,0x10
8004fe54:  sra v0,v0,0x10
8004fe58:  bne v0,v1,0x8004fea8
8004fe5c:  _nop
8004fe60:  lhu v0,0x2(a0)
8004fe64:  lh v1,0x2(s1)
8004fe68:  sll v0,v0,0x10
8004fe6c:  sra v0,v0,0x10
8004fe70:  bne v0,v1,0x8004fea8
8004fe74:  _nop
8004fe78:  lhu v0,0x4(a0)
8004fe7c:  lh v1,0x4(s1)
8004fe80:  sll v0,v0,0x10
8004fe84:  sra v0,v0,0x10
8004fe88:  bne v0,v1,0x8004fea8
8004fe8c:  _nop
8004fe90:  lhu v0,0x6(a0)
8004fe94:  lh v1,0x6(s1)
8004fe98:  sll v0,v0,0x10
8004fe9c:  sra v0,v0,0x10
8004fea0:  beq v0,v1,0x8004ff94
8004fea4:  _nop
8004fea8:  jal 0x8004f1e8
8004feac:  _nop
8004feb0:  sb v0,0x12(s1)
8004feb4:  andi v0,v0,0xff
8004feb8:  li v1,0x1
8004febc:  bne v0,v1,0x8004fec8
8004fec0:  _nop
8004fec4:  ori s0,s0,0x8
8004fec8:  lbu v0,0x11(s1)
8004fecc:  nop
8004fed0:  beq v0,zero,0x8004fedc
8004fed4:  _nop
8004fed8:  ori s0,s0,0x10
8004fedc:  lbu v0,0x10(s1)
8004fee0:  nop
8004fee4:  beq v0,zero,0x8004fef0
8004fee8:  _nop
8004feec:  ori s0,s0,0x20
8004fef0:  lui v0,0x8006
8004fef4:  lbu v0,0x5027(v0)
8004fef8:  nop
8004fefc:  beq v0,zero,0x8004ff08
8004ff00:  _nop
8004ff04:  ori s0,s0,0x80
8004ff08:  lh v1,0x4(s1)
8004ff0c:  nop
8004ff10:  slti v0,v1,0x119
8004ff14:  bne v0,zero,0x8004ff50
8004ff18:  _slti v0,v1,0x161
8004ff1c:  beq v0,zero,0x8004ff2c
8004ff20:  _slti v0,v1,0x191
8004ff24:  j 0x8004ff50
8004ff28:  _ori s0,s0,0x1
8004ff2c:  beq v0,zero,0x8004ff3c
8004ff30:  _slti v0,v1,0x231
8004ff34:  j 0x8004ff50
8004ff38:  _ori s0,s0,0x40
8004ff3c:  beq v0,zero,0x8004ff4c
8004ff40:  _nop
8004ff44:  j 0x8004ff50
8004ff48:  _ori s0,s0,0x2
8004ff4c:  ori s0,s0,0x3
