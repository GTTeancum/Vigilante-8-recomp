# addr: 0x80019960  name: FUN_80019960
80019960:  addiu sp,sp,-0x58
80019964:  sw s0,0x38(sp)
80019968:  move s0,a1
8001996c:  sw s2,0x40(sp)
80019970:  move s2,a2
80019974:  sw ra,0x50(sp)
80019978:  sw s5,0x4c(sp)
8001997c:  sw s4,0x48(sp)
80019980:  sw s3,0x44(sp)
80019984:  sw s1,0x3c(sp)
80019988:  lbu a2,0x0(s0)
8001998c:  move s3,a0
80019990:  move s4,a3
80019994:  addiu s0,s0,0x1
80019998:  beq a2,zero,0x80019a34
8001999c:  _move s5,s2
800199a0:  andi s1,a2,0xff
800199a4:  li v0,0xa
800199a8:  bne s1,v0,0x800199c8
800199ac:  _li v0,0x1
800199b0:  lw v0,0x0(s3)
800199b4:  nop
800199b8:  lbu v0,0x7(v0)
800199bc:  move s2,s5
800199c0:  j 0x80019a24
800199c4:  _addu s4,s4,v0
800199c8:  bne s1,v0,0x800199f8
800199cc:  _nop
800199d0:  lbu v0,0x0(s0)
800199d4:  nop
800199d8:  sb v0,0x4(s3)
800199dc:  lbu v0,0x1(s0)
800199e0:  nop
800199e4:  sb v0,0x5(s3)
800199e8:  lbu v0,0x2(s0)
800199ec:  addiu s0,s0,0x3
800199f0:  j 0x80019a24
800199f4:  _sb v0,0x6(s3)
800199f8:  jal 0x8004f580
800199fc:  _clear a0
80019a00:  sw s4,0x10(sp)
80019a04:  move a0,s3
80019a08:  addiu a1,sp,0x18
80019a0c:  move a2,s1
80019a10:  jal 0x80019370
80019a14:  _move a3,s2
80019a18:  move s2,v0
80019a1c:  jal 0x8004fb18
80019a20:  _addiu a0,sp,0x18
80019a24:  lbu a2,0x0(s0)
80019a28:  nop
80019a2c:  bne a2,zero,0x800199a0
80019a30:  _addiu s0,s0,0x1
80019a34:  lw ra,0x50(sp)
80019a38:  lw s5,0x4c(sp)
80019a3c:  lw s4,0x48(sp)
80019a40:  lw s3,0x44(sp)
80019a44:  lw s2,0x40(sp)
80019a48:  lw s1,0x3c(sp)
80019a4c:  lw s0,0x38(sp)
80019a50:  jr ra
80019a54:  _addiu sp,sp,0x58
