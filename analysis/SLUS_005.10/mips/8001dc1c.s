# addr: 0x8001dc1c  name: FUN_8001dc1c
8001dc1c:  addiu sp,sp,-0x28
8001dc20:  sw s3,0x1c(sp)
8001dc24:  move s3,a0
8001dc28:  sw ra,0x20(sp)
8001dc2c:  sw s2,0x18(sp)
8001dc30:  sw s1,0x14(sp)
8001dc34:  sw s0,0x10(sp)
8001dc38:  lw v1,0x30(s3)
8001dc3c:  nop
8001dc40:  beq v1,zero,0x8001dc5c
8001dc44:  _clear s2
8001dc48:  lhu v0,0x26(v1)
8001dc4c:  lhu a0,0x24(v1)
8001dc50:  li v1,0x10
8001dc54:  subu v1,v1,v0
8001dc58:  sllv s2,a0,v1
8001dc5c:  lw s1,0x38(s3)
8001dc60:  nop
8001dc64:  beq s1,zero,0x8001dca4
8001dc68:  _nop
8001dc6c:  jal 0x8001dc1c
8001dc70:  _move a0,s1
8001dc74:  addiu a0,s1,0x24
8001dc78:  jal 0x80016a20
8001dc7c:  _move s0,v0
8001dc80:  addu s0,s0,v0
8001dc84:  slt v0,s0,s2
8001dc88:  beq v0,zero,0x8001dc94
8001dc8c:  _nop
8001dc90:  move s0,s2
8001dc94:  lw s1,0x34(s1)
8001dc98:  nop
8001dc9c:  bne s1,zero,0x8001dc6c
8001dca0:  _move s2,s0
8001dca4:  sw s2,0x54(s3)
8001dca8:  lw ra,0x20(sp)
8001dcac:  lw s3,0x1c(sp)
8001dcb0:  move v0,s2
8001dcb4:  lw s2,0x18(sp)
8001dcb8:  lw s1,0x14(sp)
8001dcbc:  lw s0,0x10(sp)
8001dcc0:  jr ra
8001dcc4:  _addiu sp,sp,0x28
