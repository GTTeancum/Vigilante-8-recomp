# addr: 0x8004b904  name: StClearRing
8004b904:  addiu sp,sp,-0x18
8004b908:  lui a1,0x800a
8004b90c:  lw a1,0x32cc(a1)
8004b910:  sw ra,0x10(sp)
8004b914:  lui at,0x800a
8004b918:  sw zero,0x32b4(at)
8004b91c:  lui at,0x800a
8004b920:  sw zero,0x32b0(at)
8004b924:  lui at,0x800a
8004b928:  sw zero,0x32ac(at)
8004b92c:  lui at,0x800a
8004b930:  sw zero,0x32a4(at)
8004b934:  jal 0x8004b9f4
8004b938:  _clear a0
8004b93c:  lui at,0x800a
8004b940:  sw zero,0x3294(at)
8004b944:  lui at,0x800a
8004b948:  sh zero,0x328c(at)
8004b94c:  lui at,0x800a
8004b950:  sw zero,0x3288(at)
8004b954:  lw ra,0x10(sp)
8004b958:  addiu sp,sp,0x18
8004b95c:  jr ra
8004b960:  _nop
