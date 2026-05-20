# addr: 0x8004ad14  name: CDREAD_OBJ_0
8004ad14:  addiu sp,sp,-0x30
8004ad18:  sw s1,0x24(sp)
8004ad1c:  move s1,a1
8004ad20:  lui a1,0x8006
8004ad24:  addiu a1,a1,0x398
8004ad28:  andi a0,a0,0xff
8004ad2c:  li v0,0x1
8004ad30:  sw ra,0x28(sp)
8004ad34:  sw s0,0x20(sp)
8004ad38:  sw s1,0x34(a1)
8004ad3c:  bne a0,v0,0x8004ae68
8004ad40:  _li v0,-0x1
8004ad44:  lw v0,0x14(a1)
8004ad48:  nop
8004ad4c:  blez v0,0x8004ae6c
8004ad50:  _nop
8004ad54:  lw v1,0x10(a1)
8004ad58:  li v0,0x200
8004ad5c:  bne v1,v0,0x8004ade8
8004ad60:  _nop
8004ad64:  lw v0,0x30(a1)
8004ad68:  nop
8004ad6c:  andi v0,v0,0x1
8004ad70:  beq v0,zero,0x8004ada8
8004ad74:  _addiu a0,sp,0x10
8004ad78:  jal 0x800493ec
8004ad7c:  _clear a0
8004ad80:  addiu a0,sp,0x10
8004ad84:  jal 0x800493cc
8004ad88:  _li a1,0x3
8004ad8c:  jal 0x80049410
8004ad90:  _clear a0
8004ad94:  lui a0,0x8005
8004ad98:  jal 0x800493ec
8004ad9c:  _addiu a0,a0,-0x508c
8004ada0:  j 0x8004adb0
8004ada4:  _nop
8004ada8:  jal 0x800493ac
8004adac:  _li a1,0x3
8004ae68:  sw v0,0x14(a1)
