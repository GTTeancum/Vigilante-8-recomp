# addr: 0x80011adc  name: FUN_80011adc
80011adc:  addiu sp,sp,-0x18
80011ae0:  sw ra,0x14(sp)
80011ae4:  jal 0x80015948
80011ae8:  _sw s0,0x10(sp)
80011aec:  move s0,v0
80011af0:  beq s0,zero,0x80011bd0
80011af4:  _nop
80011af8:  lw v0,0x0(s0)
80011afc:  nop
80011b00:  addu a1,s0,v0
80011b04:  lw v1,0x0(a1)
80011b08:  li v0,-0x1
80011b0c:  beq v1,v0,0x80011bc4
80011b10:  _move a2,s0
80011b14:  li t4,-0x4
80011b18:  li t3,0x1
80011b1c:  li t2,0x2
80011b20:  li t1,0x3
80011b24:  ori t0,zero,0x8000
80011b28:  sll v0,s0,0x4
80011b2c:  srl a3,v0,0x6
80011b30:  lw v1,0x0(a1)
80011b34:  addiu a1,a1,0x4
80011b38:  and v0,v1,t4
80011b3c:  andi v1,v1,0x3
80011b40:  beq v1,t3,0x80011b74
80011b44:  _addu a0,a2,v0
80011b48:  beq v1,zero,0x80011b68
80011b4c:  _nop
80011b50:  beq v1,t2,0x80011b90
80011b54:  _nop
80011b58:  beq v1,t1,0x80011ba4
80011b5c:  _nop
80011b60:  j 0x80011bb4
80011b64:  _nop
80011b68:  lw v0,0x0(a0)
80011b6c:  j 0x80011bb0
80011b70:  _addu v0,v0,a2
80011b74:  lw v0,0x0(a1)
80011b78:  addiu a1,a1,0x4
80011b7c:  addu v0,a2,v0
80011b80:  addu v0,v0,t0
80011b84:  srl v0,v0,0x10
80011b88:  j 0x80011bb4
80011b8c:  _sh v0,0x0(a0)
80011b90:  lhu v0,0x0(a0)
80011b94:  nop
80011b98:  addu v0,v0,a2
80011b9c:  j 0x80011bb4
80011ba0:  _sh v0,0x0(a0)
80011ba4:  lw v0,0x0(a0)
80011ba8:  nop
80011bac:  addu v0,v0,a3
80011bb0:  sw v0,0x0(a0)
80011bb4:  lw v1,0x0(a1)
80011bb8:  li v0,-0x1
80011bbc:  bne v1,v0,0x80011b38
80011bc0:  _addiu a1,a1,0x4
80011bc4:  lw a1,0x0(s0)
80011bc8:  jal 0x80045134
80011bcc:  _move a0,s0
80011bd0:  lw ra,0x14(sp)
80011bd4:  move v0,s0
80011bd8:  lw s0,0x10(sp)
80011bdc:  jr ra
80011be0:  _addiu sp,sp,0x18
