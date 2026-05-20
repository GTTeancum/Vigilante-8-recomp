# addr: 0x8001e120  name: FUN_8001e120
8001e120:  lw v0,0x64(a0)
8001e124:  nop
8001e128:  beq v0,zero,0x8001e15c
8001e12c:  _nop
8001e130:  lui at,0x1f80
8001e134:  sw ra,0x3f8(at)
8001e138:  sw sp,0x3fc(at)
8001e13c:  lw sp,0x3f4(at)
8001e140:  jalr v0
8001e144:  _nop
8001e148:  lui at,0x1f80
8001e14c:  lw ra,0x3f8(at)
8001e150:  nop
8001e154:  jr ra
8001e158:  _lw sp,0x3fc(at)
8001e15c:  jr ra
8001e160:  _clear v0
