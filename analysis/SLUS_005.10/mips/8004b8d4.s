# addr: 0x8004b8d4  name: StSetRing
8004b8d4:  addiu sp,sp,-0x18
8004b8d8:  sw ra,0x10(sp)
8004b8dc:  lui at,0x800a
8004b8e0:  sw a0,0x32c8(at)
8004b8e4:  lui at,0x800a
8004b8e8:  jal 0x8004b904
8004b8ec:  _sw a1,0x32cc(at)
8004b8f0:  lw ra,0x10(sp)
8004b8f4:  addiu sp,sp,0x18
8004b8f8:  jr ra
8004b8fc:  _nop
