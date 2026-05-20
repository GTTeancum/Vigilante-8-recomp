# addr: 0x80019c0c  name: FUN_80019c0c
80019c0c:  addiu sp,sp,-0x18
80019c10:  sw s0,0x10(sp)
80019c14:  lw s0,0x28(sp)
80019c18:  sw ra,0x14(sp)
80019c1c:  jal 0x80019458
80019c20:  _nop
80019c24:  lw a1,0x0(v0)
80019c28:  lw a2,0x0(s0)
80019c2c:  addiu v1,v0,0x4
80019c30:  sll v1,v1,0x8
80019c34:  srl v1,v1,0x8
80019c38:  sw v1,0x0(s0)
80019c3c:  lbu v1,0x3(a1)
80019c40:  move a0,v0
80019c44:  sll v1,v1,0x18
80019c48:  or v1,v1,a2
80019c4c:  jal 0x800118b4
80019c50:  _sw v1,0x0(a1)
80019c54:  lw ra,0x14(sp)
80019c58:  lw s0,0x10(sp)
80019c5c:  jr ra
80019c60:  _addiu sp,sp,0x18
