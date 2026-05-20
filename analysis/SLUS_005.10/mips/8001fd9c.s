# addr: 0x8001fd9c  name: FUN_8001fd9c
8001fd9c:  lw a1,0x79c(gp)
8001fda0:  nop
8001fda4:  lw v1,0x0(a1)
8001fda8:  nop
8001fdac:  beq v1,zero,0x8001fdd8
8001fdb0:  _nop
8001fdb4:  lh v0,0xa(a1)
8001fdb8:  nop
8001fdbc:  beq v0,a0,0x8001fde0
8001fdc0:  _move v0,a1
8001fdc4:  move a1,v1
8001fdc8:  lw v1,0x0(v1)
8001fdcc:  nop
8001fdd0:  bne v1,zero,0x8001fdb4
8001fdd4:  _nop
8001fdd8:  jr ra
8001fddc:  _clear v0
8001fde0:  jr ra
8001fde4:  _nop
