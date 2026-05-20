# addr: 0x8003fc50  name: FUN_8003fc50
8003fc50:  addiu sp,sp,-0x18
8003fc54:  sw s0,0x10(sp)
8003fc58:  sw ra,0x14(sp)
8003fc5c:  jal 0x8003fbc8
8003fc60:  _move s0,a0
8003fc64:  bne v0,zero,0x8003fc74
8003fc68:  _move a0,s0
8003fc6c:  j 0x8003fc84
8003fc70:  _clear v0
8003fc74:  lw a1,0x58(s0)
8003fc78:  jal 0x8003fac4
8003fc7c:  _andi a2,v0,0xffff
8003fc80:  li v0,0x1
8003fc84:  lw ra,0x14(sp)
8003fc88:  lw s0,0x10(sp)
8003fc8c:  jr ra
8003fc90:  _addiu sp,sp,0x18
