# addr: 0x80019c64  name: FUN_80019c64
80019c64:  addiu sp,sp,-0x18
80019c68:  sw s0,0x10(sp)
80019c6c:  lw s0,0x28(sp)
80019c70:  sw ra,0x14(sp)
80019c74:  jal 0x800197f4
80019c78:  _nop
80019c7c:  lw a1,0x0(v0)
80019c80:  lw a2,0x0(s0)
80019c84:  addiu v1,v0,0x4
80019c88:  sll v1,v1,0x8
80019c8c:  srl v1,v1,0x8
80019c90:  sw v1,0x0(s0)
80019c94:  lbu v1,0x3(a1)
80019c98:  move a0,v0
80019c9c:  sll v1,v1,0x18
80019ca0:  or v1,v1,a2
80019ca4:  jal 0x800118b4
80019ca8:  _sw v1,0x0(a1)
80019cac:  lw ra,0x14(sp)
80019cb0:  lw s0,0x10(sp)
80019cb4:  jr ra
80019cb8:  _addiu sp,sp,0x18
