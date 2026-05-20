# addr: 0x80050b14  name: SYS_OBJ_1910
80050b14:  addiu sp,sp,-0x20
80050b18:  sw s0,0x10(sp)
80050b1c:  move s0,a1
80050b20:  lui a1,0x8006
80050b24:  lw a1,0x5148(a1)
80050b28:  sw ra,0x18(sp)
80050b2c:  sw s1,0x14(sp)
80050b30:  lw v0,0x0(a1)
80050b34:  lui v1,0x800
80050b38:  or v0,v0,v1
80050b3c:  sw v0,0x0(a1)
80050b40:  lui v0,0x8006
80050b44:  lw v0,0x5144(v0)
80050b48:  nop
80050b4c:  sw zero,0x0(v0)
80050b50:  sll v0,s0,0x2
80050b54:  addiu v0,v0,-0x4
80050b58:  lui v1,0x8006
80050b5c:  lw v1,0x513c(v1)
80050b60:  addu a0,a0,v0
80050b64:  sw a0,0x0(v1)
80050b68:  lui v0,0x8006
80050b6c:  lw v0,0x5140(v0)
80050b70:  lui v1,0x1100
80050b74:  sw s0,0x0(v0)
80050b78:  lui v0,0x8006
80050b7c:  lw v0,0x5144(v0)
80050b80:  ori v1,v1,0x2
80050b84:  jal 0x80051b90
80050b88:  _sw v1,0x0(v0)
80050b8c:  lui v0,0x8006
80050b90:  lw v0,0x5144(v0)
80050b94:  nop
80050b98:  lw v0,0x0(v0)
80050b9c:  lui v1,0x100
80050ba0:  and v0,v0,v1
80050ba4:  beq v0,zero,0x80050be0
80050ba8:  _move v0,s0
80050bac:  lui s1,0x100
80050bb0:  jal 0x80051bc4
80050bb4:  _nop
80050bb8:  bne v0,zero,0x80050be0
80050bbc:  _li v0,-0x1
80050bc0:  lui v0,0x8006
80050bc4:  lw v0,0x5144(v0)
80050bc8:  nop
80050bcc:  lw v0,0x0(v0)
80050bd0:  nop
80050bd4:  and v0,v0,s1
80050bd8:  bne v0,zero,0x80050bb0
80050bdc:  _move v0,s0
80050be0:  lw ra,0x18(sp)
80050be4:  lw s1,0x14(sp)
80050be8:  lw s0,0x10(sp)
80050bec:  jr ra
80050bf0:  _addiu sp,sp,0x20
