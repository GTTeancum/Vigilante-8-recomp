# addr: 0x80052ea0  name: _putchar_flash
80052ea0:  lui a2,0x8006
80052ea4:  lw a2,0x5208(a2)
80052ea8:  addiu sp,sp,-0x18
80052eac:  blez a2,0x80052ecc
80052eb0:  _sw ra,0x10(sp)
80052eb4:  lui a1,0x800a
80052eb8:  addiu a1,a1,0x4c48
80052ebc:  jal 0x80053a44
80052ec0:  _li a0,0x1
80052ec4:  lui at,0x8006
80052ec8:  sw zero,0x5208(at)
80052ecc:  lw ra,0x10(sp)
80052ed0:  addiu sp,sp,0x18
80052ed4:  jr ra
80052ed8:  _nop
