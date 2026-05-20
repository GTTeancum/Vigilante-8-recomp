# addr: 0x8001ff58  name: FUN_8001ff58
8001ff58:  lw a0,0x0(a0)
8001ff5c:  nop
8001ff60:  lw v1,0x0(a0)
8001ff64:  nop
8001ff68:  beq v1,zero,0x8001ffa4
8001ff6c:  _nop
8001ff70:  lw v0,0x8(a0)
8001ff74:  nop
8001ff78:  beq v0,a2,0x8001ff90
8001ff7c:  _nop
8001ff80:  lh v0,0x6(v0)
8001ff84:  nop
8001ff88:  beq v0,a1,0x8001ffac
8001ff8c:  _move v0,a0
8001ff90:  move a0,v1
8001ff94:  lw v1,0x0(v1)
8001ff98:  nop
8001ff9c:  bne v1,zero,0x8001ff70
8001ffa0:  _nop
8001ffa4:  jr ra
8001ffa8:  _clear v0
8001ffac:  jr ra
8001ffb0:  _nop
