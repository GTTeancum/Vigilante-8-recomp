# addr: 0x8010ad7c  name: FUN_8010ad7c
8010ad7c:  addiu sp,sp,-0x28
8010ad80:  sw ra,0x24(sp)
8010ad84:  sw s4,0x20(sp)
8010ad88:  sw s3,0x1c(sp)
8010ad8c:  sw s2,0x18(sp)
8010ad90:  sw s1,0x14(sp)
8010ad94:  sltiu a0,zero,0x2985
8010ad98:  sw s0,0x10(sp)
8010ad9c:  clear s1
8010ada0:  move s2,s1
8010ada4:  lui s3,0x8011
8010ada8:  move s0,s1
8010adac:  lw a0,0x33a0(s3)
8010adb0:  lui a1,0x8010
8010adb4:  addiu a1,a1,0xd1c
8010adb8:  li a2,0xc
8010adbc:  jal 0x80052384
8010adc0:  _addu a0,a0,s0
8010adc4:  bne v0,zero,0x8010adf8
8010adc8:  _nop
8010adcc:  lw a0,0x33a0(s3)
8010add0:  nop
8010add4:  addu a0,a0,s0
8010add8:  sltiu a0,zero,0x3b18
8010addc:  addiu a0,a0,0xd
8010ade0:  addiu v1,v0,0x1
8010ade4:  sltu v0,s2,v1
8010ade8:  beq v0,zero,0x8010adf8
8010adec:  _nop
8010adf0:  move s2,v1
8010adf4:  move s4,s1
8010adf8:  addiu s1,s1,0x1
8010adfc:  sltiu v0,s1,0xf
8010ae00:  bne v0,zero,0x8010adac
8010ae04:  _addiu s0,s0,0x28
8010ae08:  beq s2,zero,0x8010ae18
8010ae0c:  _clear a0
8010ae10:  sltiu a0,zero,0x29c1
8010ae14:  move a1,s4
8010ae18:  sltiu a0,zero,0x29b1
8010ae1c:  nop
8010ae20:  lw ra,0x24(sp)
8010ae24:  lw s4,0x20(sp)
8010ae28:  lw s3,0x1c(sp)
8010ae2c:  lw s2,0x18(sp)
8010ae30:  lw s1,0x14(sp)
8010ae34:  lw s0,0x10(sp)
8010ae38:  jr ra
8010ae3c:  _addiu sp,sp,0x28
