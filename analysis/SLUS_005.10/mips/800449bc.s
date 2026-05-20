# addr: 0x800449bc  name: FUN_800449bc
800449bc:  addiu sp,sp,-0x38
800449c0:  sw s0,0x30(sp)
800449c4:  move s0,a0
800449c8:  lui a0,0x8007
800449cc:  addiu a0,a0,-0x980
800449d0:  move a1,s0
800449d4:  sw ra,0x34(sp)
800449d8:  jal 0x80043408
800449dc:  _addiu a2,sp,0x10
800449e0:  lw v0,0x10(gp)
800449e4:  nop
800449e8:  bne v0,zero,0x80044a28
800449ec:  _lui a0,0x8007
800449f0:  lbu v0,0x5a8(gp)
800449f4:  nop
800449f8:  beq v0,zero,0x80044a18
800449fc:  _nop
80044a00:  jal 0x80044890
80044a04:  _addiu a0,sp,0x10
80044a08:  sll v0,v0,0x10
80044a0c:  sra v1,v0,0x10
80044a10:  j 0x80044ab8
80044a14:  _addu v0,v1,v0
80044a18:  jal 0x800448ec
80044a1c:  _addiu a0,sp,0x10
80044a20:  j 0x80044ab8
80044a24:  _nop
80044a28:  addiu a0,a0,-0x960
80044a2c:  move a1,s0
80044a30:  addiu s0,sp,0x20
80044a34:  jal 0x80043408
80044a38:  _move a2,s0
80044a3c:  lbu v0,0x5a8(gp)
80044a40:  nop
80044a44:  beq v0,zero,0x80044a94
80044a48:  _nop
80044a4c:  jal 0x80044890
80044a50:  _addiu a0,sp,0x10
80044a54:  move a0,s0
80044a58:  jal 0x80044890
80044a5c:  _move s0,v0
80044a60:  lh v1,0x8e4(gp)
80044a64:  sll s0,s0,0x10
80044a68:  sra s0,s0,0x10
80044a6c:  sll v0,v0,0x10
80044a70:  sra v0,v0,0x10
80044a74:  addu s0,s0,v0
80044a78:  slt v0,s0,v1
80044a7c:  beq v0,zero,0x80044a88
80044a80:  _nop
80044a84:  move v1,s0
80044a88:  sll v0,v1,0x10
80044a8c:  j 0x80044ab8
80044a90:  _addu v0,v0,v1
80044a94:  jal 0x80044890
80044a98:  _addiu a0,sp,0x10
80044a9c:  move a0,s0
80044aa0:  jal 0x80044890
80044aa4:  _move s0,v0
80044aa8:  sll s0,s0,0x10
80044aac:  sll v0,v0,0x10
80044ab0:  sra v0,v0,0x10
80044ab4:  or v0,s0,v0
80044ab8:  lw ra,0x34(sp)
80044abc:  lw s0,0x30(sp)
80044ac0:  jr ra
80044ac4:  _addiu sp,sp,0x38
