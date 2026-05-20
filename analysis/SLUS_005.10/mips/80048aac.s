# addr: 0x80048aac  name: INTR_DMA_OBJ_278
80048aac:  beq a1,zero,0x80048ac8
80048ab0:  _addiu v0,a1,-0x1
80048ab4:  li v1,-0x1
80048ab8:  sw zero,0x0(a0)
80048abc:  addiu v0,v0,-0x1
80048ac0:  bne v0,v1,0x80048ab8
80048ac4:  _addiu a0,a0,0x4
80048ac8:  jr ra
80048acc:  _nop
