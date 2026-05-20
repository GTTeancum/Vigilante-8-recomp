# addr: 0x80051a20  name: SYS_OBJ_281C
80051a20:  lui a0,0x8006
80051a24:  lw a0,0x515c(a0)
80051a28:  jal 0x800481d0
80051a2c:  _nop
80051a30:  andi v0,s0,0x7
80051a34:  bne v0,zero,0x80051a44
80051a38:  _clear v0
80051a3c:  jal 0x80051d08
80051a40:  _move a0,s0
80051a44:  lw ra,0x14(sp)
80051a48:  lw s0,0x10(sp)
80051a4c:  jr ra
80051a50:  _addiu sp,sp,0x18
