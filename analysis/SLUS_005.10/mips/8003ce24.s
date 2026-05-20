# addr: 0x8003ce24  name: FUN_8003ce24
8003ce24:  lb v0,0x16(gp)
8003ce28:  addiu sp,sp,-0x28
8003ce2c:  sw s1,0x14(sp)
8003ce30:  sw ra,0x20(sp)
8003ce34:  sw s3,0x1c(sp)
8003ce38:  sw s2,0x18(sp)
8003ce3c:  sw s0,0x10(sp)
8003ce40:  nor s0,zero,v0
8003ce44:  addiu s0,s0,0x3
8003ce48:  li v0,-0x1
8003ce4c:  beq s0,v0,0x8003cec0
8003ce50:  _move s1,a0
8003ce54:  lui v0,0x8006
8003ce58:  addiu s3,v0,-0x137c
8003ce5c:  li s2,0x1
8003ce60:  jal 0x80017160
8003ce64:  _nop
8003ce68:  sll v1,v0,0x1
8003ce6c:  addu v1,v1,v0
8003ce70:  sll v1,v1,0x2
8003ce74:  srl v1,v1,0xf
8003ce78:  sll v0,v1,0x1
8003ce7c:  addu v0,v0,s3
8003ce80:  lh v0,0x0(v0)
8003ce84:  nop
8003ce88:  bltz v0,0x8003ce60
8003ce8c:  _addiu v0,v1,0x13
8003ce90:  sllv v0,s2,v0
8003ce94:  and v0,s1,v0
8003ce98:  beq v0,zero,0x8003ce60
8003ce9c:  _li v0,0xb
8003cea0:  beq v1,v0,0x8003cec0
8003cea4:  _lui v0,0x4000
8003cea8:  and v0,s1,v0
8003ceac:  beq v0,zero,0x8003cec0
8003ceb0:  _addiu s0,s0,-0x1
8003ceb4:  li v0,-0x1
8003ceb8:  bne s0,v0,0x8003ce60
8003cebc:  _nop
8003cec0:  lw ra,0x20(sp)
8003cec4:  lw s3,0x1c(sp)
8003cec8:  lw s2,0x18(sp)
8003cecc:  lw s1,0x14(sp)
8003ced0:  lw s0,0x10(sp)
8003ced4:  move v0,v1
8003ced8:  jr ra
8003cedc:  _addiu sp,sp,0x28
