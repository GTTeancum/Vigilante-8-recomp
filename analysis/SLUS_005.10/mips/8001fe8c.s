# addr: 0x8001fe8c  name: FUN_8001fe8c
8001fe8c:  lw a0,0x0(a0)
8001fe90:  nop
8001fe94:  lw v1,0x0(a0)
8001fe98:  nop
8001fe9c:  beq v1,zero,0x8001ff04
8001fea0:  _nop
8001fea4:  lui a2,0x8006
8001fea8:  addiu a2,a2,0x5a74
8001feac:  lw v0,0x8(a0)
8001feb0:  nop
8001feb4:  bne v0,a1,0x8001fef0
8001feb8:  _nop
8001febc:  lw v0,0x4(a0)
8001fec0:  lw v1,0x0(a0)
8001fec4:  nop
8001fec8:  sw v0,0x4(v1)
8001fecc:  sw v1,0x0(v0)
8001fed0:  lw v1,0x774(gp)
8001fed4:  li v0,0x1
8001fed8:  sw a0,0x774(gp)
8001fedc:  sw a0,0x0(v1)
8001fee0:  sw v1,0x4(a0)
8001fee4:  sw a2,0x0(a0)
8001fee8:  jr ra
8001feec:  _sw zero,0x8(a0)
8001fef0:  move a0,v1
8001fef4:  lw v1,0x0(v1)
8001fef8:  nop
8001fefc:  bne v1,zero,0x8001feac
8001ff00:  _nop
8001ff04:  jr ra
8001ff08:  _clear v0
