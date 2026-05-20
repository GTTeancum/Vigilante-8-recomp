# addr: 0x8002ca94  name: FUN_8002ca94
8002ca94:  addiu sp,sp,-0x28
8002ca98:  sw s4,0x20(sp)
8002ca9c:  move s4,a0
8002caa0:  sw s1,0x14(sp)
8002caa4:  sw s3,0x1c(sp)
8002caa8:  addiu s3,s4,0xa4
8002caac:  sw ra,0x24(sp)
8002cab0:  sw s2,0x18(sp)
8002cab4:  sw s0,0x10(sp)
8002cab8:  lbu s2,0xf(s3)
8002cabc:  jal 0x8002c99c
8002cac0:  _move s1,a1
8002cac4:  sll v0,s1,0x2
8002cac8:  addu s0,v0,s4
8002cacc:  addiu a0,s0,0x110
8002cad0:  addiu a1,s0,0x114
8002cad4:  li a2,0x2
8002cad8:  subu a2,a2,s1
8002cadc:  jal 0x80044c44
8002cae0:  _sll a2,a2,0x2
8002cae4:  slt v0,s1,s2
8002cae8:  bne v0,zero,0x8002cb10
8002caec:  _sw zero,0x118(s4)
8002caf0:  beq s1,zero,0x8002cb18
8002caf4:  _nop
8002caf8:  bne s2,s1,0x8002cb5c
8002cafc:  _nop
8002cb00:  lw v0,0x110(s0)
8002cb04:  nop
8002cb08:  bne v0,zero,0x8002cb18
8002cb0c:  _nop
8002cb10:  addiu v0,s2,-0x1
8002cb14:  sb v0,0xf(s3)
8002cb18:  bne s2,s1,0x8002cb5c
8002cb1c:  _nop
8002cb20:  lbu v0,0xf(s3)
8002cb24:  nop
8002cb28:  sll v0,v0,0x2
8002cb2c:  addu v0,s4,v0
8002cb30:  lw a0,0x110(v0)
8002cb34:  nop
8002cb38:  beq a0,zero,0x8002cb5c
8002cb3c:  _li v1,0x1e
8002cb40:  lh a1,0x6(a0)
8002cb44:  nop
8002cb48:  slt v0,v1,a1
8002cb4c:  beq v0,zero,0x8002cb58
8002cb50:  _nop
8002cb54:  move v1,a1
8002cb58:  sh v1,0x6(a0)
8002cb5c:  lw ra,0x24(sp)
8002cb60:  lw s4,0x20(sp)
8002cb64:  lw s3,0x1c(sp)
8002cb68:  lw s2,0x18(sp)
8002cb6c:  lw s1,0x14(sp)
8002cb70:  lw s0,0x10(sp)
8002cb74:  jr ra
8002cb78:  _addiu sp,sp,0x28
