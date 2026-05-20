# addr: 0x80048804  name: INTR_VB_OBJ_F0
80048804:  beq a1,zero,0x80048820
80048808:  _addiu v0,a1,-0x1
8004880c:  li v1,-0x1
80048810:  sw zero,0x0(a0)
80048814:  addiu v0,v0,-0x1
80048818:  bne v0,v1,0x80048810
8004881c:  _addiu a0,a0,0x4
80048820:  jr ra
80048824:  _nop
