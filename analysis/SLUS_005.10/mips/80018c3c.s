# addr: 0x80018c3c  name: FUN_80018c3c
80018c3c:  addiu sp,sp,-0x18
80018c40:  li v0,0x1
80018c44:  sw ra,0x10(sp)
80018c48:  sb v0,0x3(a0)
80018c4c:  lhu v1,0x8(a1)
80018c50:  lui a2,0xe100
80018c54:  ori a2,a2,0x400
80018c58:  li v0,0x4
80018c5c:  sb v0,0xb(a0)
80018c60:  li v0,0x65
80018c64:  sb v0,0xf(a0)
80018c68:  andi v1,v1,0x9ff
80018c6c:  or v1,v1,a2
80018c70:  sw v1,0x4(a0)
80018c74:  lhu v0,0xa(a1)
80018c78:  nop
80018c7c:  sh v0,0x16(a0)
80018c80:  lhu v0,0x2(a1)
80018c84:  nop
80018c88:  sh v0,0x18(a0)
80018c8c:  lhu v0,0x4(a1)
80018c90:  nop
80018c94:  sh v0,0x1a(a0)
80018c98:  lhu v0,0x6(a1)
80018c9c:  addiu a1,a0,0x8
80018ca0:  jal 0x80052344
80018ca4:  _sh v0,0x14(a0)
80018ca8:  lw ra,0x10(sp)
80018cac:  nop
80018cb0:  jr ra
80018cb4:  _addiu sp,sp,0x18
