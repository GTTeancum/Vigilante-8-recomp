# addr: 0x8001a0ac  name: FUN_8001a0ac
8001a0ac:  addiu sp,sp,-0x28
8001a0b0:  li v0,0x3
8001a0b4:  sb v0,0x13(sp)
8001a0b8:  li v0,0x60
8001a0bc:  sb v0,0x17(sp)
8001a0c0:  srl v0,a1,0x8
8001a0c4:  sb a1,0x14(sp)
8001a0c8:  srl a1,a1,0x10
8001a0cc:  sw ra,0x20(sp)
8001a0d0:  sb v0,0x15(sp)
8001a0d4:  sb a1,0x16(sp)
8001a0d8:  lhu v0,0x0(a0)
8001a0dc:  nop
8001a0e0:  sh v0,0x18(sp)
8001a0e4:  lhu v0,0x2(a0)
8001a0e8:  nop
8001a0ec:  sh v0,0x1a(sp)
8001a0f0:  lhu v0,0x4(a0)
8001a0f4:  nop
8001a0f8:  sh v0,0x1c(sp)
8001a0fc:  lh v0,0x6(a0)
8001a100:  addiu a0,sp,0x10
8001a104:  jal 0x8004fb18
8001a108:  _sh v0,0x1e(sp)
8001a10c:  lw ra,0x20(sp)
8001a110:  nop
8001a114:  jr ra
8001a118:  _addiu sp,sp,0x28
