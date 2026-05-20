# addr: 0x8002ce68  name: FUN_8002ce68
8002ce68:  addiu sp,sp,-0x20
8002ce6c:  sw s0,0x10(sp)
8002ce70:  move s0,a0
8002ce74:  sw ra,0x18(sp)
8002ce78:  sw s1,0x14(sp)
8002ce7c:  lbu v0,0xb3(s0)
8002ce80:  nop
8002ce84:  sll v0,v0,0x2
8002ce88:  addu v0,s0,v0
8002ce8c:  lw s1,0x110(v0)
8002ce90:  nop
8002ce94:  beq s1,zero,0x8002cf7c
8002ce98:  _nop
8002ce9c:  beq a1,zero,0x8002cf60
8002cea0:  _nop
8002cea4:  lhu v0,0xc(s1)
8002cea8:  nop
8002ceac:  beq v0,zero,0x8002cf60
8002ceb0:  _nop
8002ceb4:  lh v0,0x6(s1)
8002ceb8:  nop
8002cebc:  bne v0,zero,0x8002cf60
8002cec0:  _nop
8002cec4:  lw v0,0x64(s1)
8002cec8:  nop
8002cecc:  beq v0,zero,0x8002cee8
8002ced0:  _move a0,s1
8002ced4:  li a1,0xb
8002ced8:  jalr v0
8002cedc:  _move a2,s0
8002cee0:  j 0x8002cef0
8002cee4:  _sh v0,0x6(s1)
8002cee8:  clear v0
8002ceec:  sh v0,0x6(s1)
8002cef0:  lh v0,0x6(s0)
8002cef4:  nop
8002cef8:  blez v0,0x8002cf7c
8002cefc:  _lui a1,0x4
8002cf00:  lw v0,0x0(s1)
8002cf04:  nop
8002cf08:  and v0,v0,a1
8002cf0c:  bne v0,zero,0x8002cf7c
8002cf10:  _li a0,0x2
8002cf14:  lb v0,0x16(gp)
8002cf18:  lw v1,0x0(s0)
8002cf1c:  nop
8002cf20:  and v1,v1,a1
8002cf24:  bne v1,zero,0x8002cf34
8002cf28:  _subu v0,a0,v0
8002cf2c:  j 0x8002cf38
8002cf30:  _sll s0,v0,0x6
8002cf34:  sll s0,v0,0x5
8002cf38:  jal 0x80017160
8002cf3c:  _nop
8002cf40:  mult v0,s0
8002cf44:  lhu v1,0x6(s1)
8002cf48:  mflo a3
8002cf4c:  sra v0,a3,0xf
8002cf50:  addu v0,s0,v0
8002cf54:  addu v1,v1,v0
8002cf58:  j 0x8002cf7c
8002cf5c:  _sh v1,0x6(s1)
8002cf60:  lw v0,0x64(s1)
8002cf64:  nop
8002cf68:  beq v0,zero,0x8002cf7c
8002cf6c:  _move a0,s1
8002cf70:  clear a1
8002cf74:  jalr v0
8002cf78:  _move a2,s0
8002cf7c:  lw ra,0x18(sp)
8002cf80:  lw s1,0x14(sp)
8002cf84:  lw s0,0x10(sp)
8002cf88:  jr ra
8002cf8c:  _addiu sp,sp,0x20
