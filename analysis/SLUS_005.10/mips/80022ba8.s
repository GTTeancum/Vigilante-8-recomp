# addr: 0x80022ba8  name: FUN_80022ba8
80022ba8:  addiu sp,sp,-0x28
80022bac:  sw s1,0x14(sp)
80022bb0:  move s1,a0
80022bb4:  sw s2,0x18(sp)
80022bb8:  move s2,a1
80022bbc:  sw s3,0x1c(sp)
80022bc0:  move s3,a2
80022bc4:  lui a0,0x8006
80022bc8:  addiu a0,a0,0x5690
80022bcc:  sw ra,0x20(sp)
80022bd0:  jal 0x80011adc
80022bd4:  _sw s0,0x10(sp)
80022bd8:  move s0,v0
80022bdc:  lw v0,0x4(s0)
80022be0:  move a0,s1
80022be4:  move a1,s2
80022be8:  jalr v0
80022bec:  _move a2,s3
80022bf0:  jal 0x80045088
80022bf4:  _move a0,s0
80022bf8:  lw a0,0x6f8(gp)
80022bfc:  lw v0,0x730(gp)
80022c00:  li a1,0x1
80022c04:  jalr v0
80022c08:  _clear a2
80022c0c:  lw a1,0x6f8(gp)
80022c10:  nop
80022c14:  lw v0,0x0(a1)
80022c18:  nop
80022c1c:  andi v0,v0,0x80
80022c20:  beq v0,zero,0x80022c38
80022c24:  _nop
80022c28:  lui a0,0x8006
80022c2c:  addiu a0,a0,0x5a60
80022c30:  jal 0x8001fe50
80022c34:  _nop
80022c38:  lw ra,0x20(sp)
80022c3c:  lw s3,0x1c(sp)
80022c40:  lw s2,0x18(sp)
80022c44:  lw s1,0x14(sp)
80022c48:  lw s0,0x10(sp)
80022c4c:  jr ra
80022c50:  _addiu sp,sp,0x28
