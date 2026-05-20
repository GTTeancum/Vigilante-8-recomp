# addr: 0x80024d30  name: FUN_80024d30
80024d30:  lui at,0x1f80
80024d34:  sw ra,0x3f0(at)
80024d38:  sw sp,0x3f4(at)
80024d3c:  jal 0x80024d54
80024d40:  _addi sp,at,0x3e0
80024d44:  lw ra,0x10(sp)
80024d48:  lw sp,0x14(sp)
80024d4c:  jr ra
80024d50:  _nop
