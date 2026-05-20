# addr: 0x80022a1c  name: FUN_80022a1c
80022a1c:  lw a0,0x7d8(gp)
80022a20:  addiu sp,sp,-0x20
80022a24:  sw ra,0x18(sp)
80022a28:  sw s1,0x14(sp)
80022a2c:  jal 0x8001af48
80022a30:  _sw s0,0x10(sp)
80022a34:  lw a0,0x6f8(gp)
80022a38:  nop
80022a3c:  beq a0,zero,0x80022a54
80022a40:  _nop
80022a44:  jal 0x800203fc
80022a48:  _nop
80022a4c:  jal 0x80045088
80022a50:  _move a0,v0
80022a54:  lw a0,0x6ec(gp)
80022a58:  nop
80022a5c:  beq a0,zero,0x80022a6c
80022a60:  _nop
80022a64:  jal 0x80045088
80022a68:  _nop
80022a6c:  lui a0,0x8006
80022a70:  addiu a0,a0,0x5a50
80022a74:  jal 0x80020658
80022a78:  _nop
80022a7c:  lui a0,0x8006
80022a80:  addiu a0,a0,0x5a18
80022a84:  jal 0x80020658
80022a88:  _nop
80022a8c:  lw a0,0x6fc(gp)
80022a90:  jal 0x80020968
80022a94:  _nop
80022a98:  lw v0,0x7c4(gp)
80022a9c:  lui v1,0x8006
80022aa0:  addiu v1,v1,0x5ac0
80022aa4:  sw zero,0x6fc(gp)
80022aa8:  beq v0,v1,0x80022ad8
80022aac:  _nop
80022ab0:  move s0,v1
80022ab4:  lw v0,0x7bc(gp)
80022ab8:  nop
80022abc:  lw a0,0x8(v0)
80022ac0:  jal 0x80020540
80022ac4:  _nop
80022ac8:  lw v0,0x7c4(gp)
80022acc:  nop
80022ad0:  bne v0,s0,0x80022ab4
80022ad4:  _nop
80022ad8:  lw v0,0x7a4(gp)
80022adc:  lui v1,0x8006
80022ae0:  addiu v1,v1,0x5aa0
80022ae4:  beq v0,v1,0x80022b20
80022ae8:  _nop
80022aec:  move s0,v1
80022af0:  lw a0,0x79c(gp)
80022af4:  nop
80022af8:  lw v0,0x0(a0)
80022afc:  nop
80022b00:  sw s0,0x4(v0)
80022b04:  sw v0,0x79c(gp)
80022b08:  jal 0x80045088
80022b0c:  _nop
80022b10:  lw v0,0x7a4(gp)
80022b14:  nop
80022b18:  bne v0,s0,0x80022af0
80022b1c:  _nop
80022b20:  lw a0,0x734(gp)
80022b24:  nop
80022b28:  beq a0,zero,0x80022b3c
80022b2c:  _lui v0,0x8007
80022b30:  jal 0x80045088
80022b34:  _nop
80022b38:  lui v0,0x8007
80022b3c:  addiu a0,v0,0x37a0
80022b40:  lw v1,0x44(a0)
80022b44:  lw v0,0x40(a0)
80022b48:  nop
80022b4c:  bne v1,v0,0x80022b58
80022b50:  _clear s1
80022b54:  sw zero,0x44(a0)
80022b58:  move s0,a0
80022b5c:  lw a0,0x0(s0)
80022b60:  nop
80022b64:  beq a0,zero,0x80022b74
80022b68:  _nop
80022b6c:  jal 0x8001aa38
80022b70:  _nop
80022b74:  addiu s1,s1,0x1
80022b78:  slti v0,s1,0x40
80022b7c:  bne v0,zero,0x80022b5c
80022b80:  _addiu s0,s0,0x4
80022b84:  lui a0,0x8006
80022b88:  addiu a0,a0,0x5a28
80022b8c:  jal 0x8001884c
80022b90:  _nop
80022b94:  lw ra,0x18(sp)
80022b98:  lw s1,0x14(sp)
80022b9c:  lw s0,0x10(sp)
80022ba0:  jr ra
80022ba4:  _addiu sp,sp,0x20
