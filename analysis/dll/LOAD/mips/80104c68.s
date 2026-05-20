# addr: 0x80104c68  name: FUN_80104c68
80104c68:  addiu sp,sp,-0x28
80104c6c:  sw s0,0x20(sp)
80104c70:  move s0,a0
80104c74:  move a0,a2
80104c78:  sw ra,0x24(sp)
80104c7c:  jal 0x80043224
80104c80:  _addiu a1,sp,0x10
80104c84:  lw v1,0x10(sp)
80104c88:  nop
80104c8c:  bgez v1,0x80104c98
80104c90:  _ori v0,zero,0xffff
80104c94:  addu v1,v1,v0
80104c98:  lw a0,0x18(sp)
80104c9c:  nop
80104ca0:  bgez a0,0x80104cb0
80104ca4:  _sra a2,v1,0x10
80104ca8:  ori v0,zero,0xffff
80104cac:  addu a0,a0,v0
80104cb0:  sra a0,a0,0x10
80104cb4:  lui a1,0x8009
80104cb8:  addiu a1,a1,0x1120
80104cbc:  srl v0,a0,0x6
80104cc0:  sll v0,v0,0x2
80104cc4:  srl v1,a2,0x6
80104cc8:  sll v1,v1,0x7
80104ccc:  addu v0,v0,v1
80104cd0:  addu v0,v0,a1
80104cd4:  lw v1,0x80(v0)
80104cd8:  andi a0,a0,0x3f
80104cdc:  sll a0,a0,0x1
80104ce0:  andi v0,a2,0x3f
80104ce4:  sll v0,v0,0x7
80104ce8:  addu a0,a0,v0
80104cec:  addu v1,v1,a0
80104cf0:  lhu v0,0x0(v1)
80104cf4:  nop
80104cf8:  srl v0,v0,0xb
80104cfc:  sll v0,v0,0x2
80104d00:  sb v0,0x2(s0)
80104d04:  sb v0,0x1(s0)
80104d08:  sb v0,0x0(s0)
80104d0c:  lw ra,0x24(sp)
80104d10:  lw s0,0x20(sp)
80104d14:  jr ra
80104d18:  _addiu sp,sp,0x28
