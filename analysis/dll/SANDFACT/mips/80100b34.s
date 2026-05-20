# addr: 0x80100b34  name: FUN_80100b34
80100b34:  addiu sp,sp,-0x20
80100b38:  sw s0,0x10(sp)
80100b3c:  move s0,a0
80100b40:  li v0,0x1
80100b44:  sw ra,0x18(sp)
80100b48:  beq a1,v0,0x80100c2c
80100b4c:  _sw s1,0x14(sp)
80100b50:  beq a1,zero,0x80100b68
80100b54:  _li v0,0x2
80100b58:  beq a1,v0,0x80100bb4
80100b5c:  _nop
80100b60:  slti a0,zero,0x311
80100b64:  nop
80100b68:  jal 0x800449bc
80100b6c:  _addiu a0,s0,0x48
80100b70:  beq v0,zero,0x80100b8c
80100b74:  _nop
80100b78:  lb a0,0x5(s0)
80100b7c:  jal 0x80044574
80100b80:  _move a1,v0
80100b84:  slti a0,zero,0x316
80100b88:  clear v0
80100b8c:  jal 0x80020778
80100b90:  _move a0,s0
80100b94:  move a0,s0
80100b98:  jal 0x80020890
80100b9c:  _li a1,0x78
80100ba0:  lb a0,0x5(s0)
80100ba4:  jal 0x800441c8
80100ba8:  _nop
80100bac:  slti a0,zero,0x315
80100bb0:  sb zero,0x5(s0)
80100bb4:  jal 0x800449bc
80100bb8:  _addiu a0,s0,0x48
80100bbc:  move s1,v0
80100bc0:  beq s1,zero,0x80100c18
80100bc4:  _nop
80100bc8:  lb v0,0x5(s0)
80100bcc:  nop
80100bd0:  beq v0,zero,0x80100be0
80100bd4:  _lui a0,0x8010
80100bd8:  jal 0x80015368
80100bdc:  _addiu a0,a0,0xd0
80100be0:  jal 0x8004410c
80100be4:  _nop
80100be8:  lw v1,0x58(s0)
80100bec:  sb v0,0x5(s0)
80100bf0:  lw a1,0x8(v1)
80100bf4:  sll v0,v0,0x18
80100bf8:  sra a0,v0,0x18
80100bfc:  clear a2
80100c00:  jal 0x800443c8
80100c04:  _move a3,s1
80100c08:  jal 0x80020744
80100c0c:  _move a0,s0
80100c10:  slti a0,zero,0x316
80100c14:  clear v0
80100c18:  move a0,s0
80100c1c:  jal 0x80020890
80100c20:  _li a1,0x78
80100c24:  slti a0,zero,0x316
80100c28:  clear v0
80100c2c:  lbu a1,0x9(s0)
80100c30:  move a0,s0
80100c34:  jal 0x80020890
80100c38:  _addiu a1,a1,0x78
80100c3c:  slti a0,zero,0x316
80100c40:  clear v0
80100c44:  jal 0x800223dc
80100c48:  _move a0,s0
80100c4c:  slti a0,zero,0x316
80100c50:  nop
80100c54:  clear v0
80100c58:  lw ra,0x18(sp)
80100c5c:  lw s1,0x14(sp)
80100c60:  lw s0,0x10(sp)
80100c64:  jr ra
80100c68:  _addiu sp,sp,0x20
