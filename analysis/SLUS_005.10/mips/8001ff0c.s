# addr: 0x8001ff0c  name: FUN_8001ff0c
8001ff0c:  lw a0,0x0(a0)
8001ff10:  nop
8001ff14:  lw v1,0x0(a0)
8001ff18:  nop
8001ff1c:  beq v1,zero,0x8001ff48
8001ff20:  _nop
8001ff24:  lw v0,0x8(a0)
8001ff28:  nop
8001ff2c:  beq v0,a1,0x8001ff50
8001ff30:  _move v0,a0
8001ff34:  move a0,v1
8001ff38:  lw v1,0x0(v1)
8001ff3c:  nop
8001ff40:  bne v1,zero,0x8001ff24
8001ff44:  _nop
8001ff48:  jr ra
8001ff4c:  _clear v0
8001ff50:  jr ra
8001ff54:  _nop
