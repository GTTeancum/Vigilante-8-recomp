# addr: 0x80100c50  name: FUN_80100c50
80100c50:  addiu sp,sp,-0x18
80100c54:  sw s0,0x10(sp)
80100c58:  move s0,a0
80100c5c:  bne a1,zero,0x80100ca8
80100c60:  _sw ra,0x14(sp)
80100c64:  jal 0x80017324
80100c68:  _nop
80100c6c:  lw v0,0x0(s0)
80100c70:  nop
80100c74:  andi v0,v0,0x2
80100c78:  bne v0,zero,0x80100c90
80100c7c:  _nop
80100c80:  lw v0,0x84(s0)
80100c84:  nop
80100c88:  addiu v0,v0,-0x1680
80100c8c:  sw v0,0x84(s0)
80100c90:  lw v1,0x24(s0)
80100c94:  lw a2,0x28(s0)
80100c98:  lw a3,0x2c(s0)
80100c9c:  sw v1,0x48(s0)
80100ca0:  sw a2,0x4c(s0)
80100ca4:  sw a3,0x50(s0)
80100ca8:  lw ra,0x14(sp)
80100cac:  lw s0,0x10(sp)
80100cb0:  clear v0
80100cb4:  jr ra
80100cb8:  _addiu sp,sp,0x18
