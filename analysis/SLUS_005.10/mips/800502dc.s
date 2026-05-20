# addr: 0x800502dc  name: GetODE
800502dc:  lui v0,0x8006
800502e0:  lw v0,0x501c(v0)
800502e4:  addiu sp,sp,-0x18
800502e8:  sw ra,0x10(sp)
800502ec:  lw v0,0x38(v0)
800502f0:  nop
800502f4:  jalr v0
800502f8:  _nop
800502fc:  lw ra,0x10(sp)
80050300:  srl v0,v0,0x1f
80050304:  jr ra
80050308:  _addiu sp,sp,0x18
