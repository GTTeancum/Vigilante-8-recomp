# addr: 0x8003d898  name: FUN_8003d898
8003d898:  lw v1,0x10(gp)
8003d89c:  li a1,0x1
8003d8a0:  bne v1,a1,0x8003d8ac
8003d8a4:  _li v0,-0x100
8003d8a8:  li v0,-0xa0
8003d8ac:  sh v0,0x8c(a0)
8003d8b0:  bne v1,a1,0x8003d8bc
8003d8b4:  _li v0,0xa0
8003d8b8:  li v0,0x50
8003d8bc:  jr ra
8003d8c0:  _sh v0,0x90(a0)
