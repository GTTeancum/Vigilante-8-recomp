# addr: 0x80105a18  name: FUN_80105a18
80105a18:  addiu sp,sp,-0x38
80105a1c:  sw s1,0x2c(sp)
80105a20:  move s1,a0
80105a24:  sw s0,0x28(sp)
80105a28:  lui s0,0x8006
80105a2c:  addiu s0,s0,0x5b18
80105a30:  sw ra,0x30(sp)
80105a34:  jal 0x800187e4
80105a38:  _move a1,s0
80105a3c:  addiu a0,sp,0x10
80105a40:  jal 0x800185cc
80105a44:  _move a1,s1
80105a48:  lw a0,0x1c(sp)
80105a4c:  nop
80105a50:  lh v0,0x2(a0)
80105a54:  lh a1,0x6(a0)
80105a58:  clear a3
80105a5c:  li t1,0x9
80105a60:  lui v1,0x8006
80105a64:  addiu v1,v1,-0x164c
80105a68:  subu v0,v0,a1
80105a6c:  sll v0,v0,0x4
80105a70:  sh v0,0x12(v1)
80105a74:  sh v0,0xa(v1)
80105a78:  sh v0,0x2(v1)
80105a7c:  lh a0,0x2(a0)
80105a80:  li t0,0x2c
80105a84:  lui v0,0x8009
80105a88:  addiu v0,v0,0x1020
80105a8c:  addiu a2,v0,0x28
80105a90:  move a1,v0
80105a94:  sll a0,a0,0x4
80105a98:  sh a0,0x2a(v1)
80105a9c:  sh a0,0x22(v1)
80105aa0:  sh a0,0x1a(v1)
80105aa4:  sb t1,0x3(a1)
80105aa8:  sb t0,0x7(a1)
80105aac:  sb t1,0x3(a2)
80105ab0:  sb t0,0x7(a2)
80105ab4:  lbu v0,0x7(a1)
80105ab8:  nop
80105abc:  ori v0,v0,0x1
80105ac0:  sb v0,0x7(a1)
80105ac4:  lbu v0,0x7(a2)
80105ac8:  nop
80105acc:  ori v0,v0,0x1
80105ad0:  sb v0,0x7(a2)
80105ad4:  lhu v0,0xa(s0)
80105ad8:  nop
80105adc:  sh v0,0x36(a1)
80105ae0:  sh v0,0xe(a1)
80105ae4:  lhu v0,0x8(s0)
80105ae8:  nop
80105aec:  sh v0,0x3e(a1)
80105af0:  sh v0,0x16(a1)
80105af4:  lbu v0,0x6(s0)
80105af8:  nop
80105afc:  sb v0,0xc(a1)
80105b00:  lhu v0,0x6(s0)
80105b04:  nop
80105b08:  srl v0,v0,0x8
80105b0c:  sb v0,0xd(a1)
80105b10:  lbu v0,0x6(s0)
80105b14:  lbu v1,0x2(s0)
80105b18:  addiu v0,v0,0xff
80105b1c:  addu v1,v1,v0
80105b20:  sb v1,0x14(a1)
80105b24:  lhu v0,0x6(s0)
80105b28:  nop
80105b2c:  srl v0,v0,0x8
80105b30:  sb v0,0x15(a1)
80105b34:  lbu v0,0x6(s0)
80105b38:  nop
80105b3c:  sb v0,0x1c(a1)
80105b40:  lhu v0,0x6(s0)
80105b44:  lbu v1,0x4(s0)
80105b48:  srl v0,v0,0x8
80105b4c:  addiu v0,v0,0xff
80105b50:  addu v1,v1,v0
80105b54:  sb v1,0x1d(a1)
80105b58:  lbu v0,0x6(s0)
80105b5c:  lbu v1,0x2(s0)
80105b60:  addiu v0,v0,0xff
80105b64:  addu v1,v1,v0
80105b68:  sb v1,0x24(a1)
80105b6c:  lhu v0,0x6(s0)
80105b70:  lbu v1,0x4(s0)
80105b74:  srl v0,v0,0x8
80105b78:  addiu v0,v0,0xff
80105b7c:  addu v1,v1,v0
80105b80:  sb v1,0x25(a1)
80105b84:  lbu v0,0x6(s0)
80105b88:  nop
80105b8c:  sb v0,0xc(a2)
80105b90:  lhu v0,0x6(s0)
80105b94:  nop
80105b98:  srl v0,v0,0x8
80105b9c:  sb v0,0xd(a2)
80105ba0:  lbu v0,0x6(s0)
80105ba4:  lbu v1,0x2(s0)
80105ba8:  addiu v0,v0,0xff
80105bac:  addu v1,v1,v0
80105bb0:  sb v1,0x14(a2)
80105bb4:  lhu v0,0x6(s0)
80105bb8:  nop
80105bbc:  srl v0,v0,0x8
80105bc0:  sb v0,0x15(a2)
80105bc4:  lbu v0,0x6(s0)
80105bc8:  nop
80105bcc:  sb v0,0x1c(a2)
80105bd0:  lhu v0,0x6(s0)
80105bd4:  lbu v1,0x4(s0)
80105bd8:  srl v0,v0,0x8
80105bdc:  addiu v0,v0,0xff
80105be0:  addu v1,v1,v0
80105be4:  sb v1,0x1d(a2)
80105be8:  lbu v0,0x6(s0)
80105bec:  lbu v1,0x2(s0)
80105bf0:  addiu v0,v0,0xff
80105bf4:  addu v1,v1,v0
80105bf8:  sb v1,0x24(a2)
80105bfc:  lhu v0,0x6(s0)
80105c00:  lbu v1,0x4(s0)
80105c04:  addiu a3,a3,0x1
80105c08:  addiu a1,a1,0x50
80105c0c:  srl v0,v0,0x8
80105c10:  addiu v0,v0,0xff
80105c14:  addu v1,v1,v0
80105c18:  sb v1,0x25(a2)
80105c1c:  slti v0,a3,0x2
80105c20:  bne v0,zero,0x80105aa4
80105c24:  _addiu a2,a2,0x50
80105c28:  lui a1,0x8006
80105c2c:  lhu v0,0x5a28(a1)
80105c30:  nop
80105c34:  bne v0,zero,0x80105c88
80105c38:  _lui v1,0x8006
80105c3c:  addiu v0,v1,0x5b18
80105c40:  lbu a0,0x4(v0)
80105c44:  lui v0,0x8006
80105c48:  addiu t5,v1,0x5b18
80105c4c:  addiu t4,a1,0x5a28
80105c50:  lwl t2,0x3(t5)
80105c54:  lwr t2,0x0(t5)
80105c58:  lwl t3,0x7(t5)
80105c5c:  lwr t3,0x4(t5)
80105c60:  swl t2,0x3(t4)
80105c64:  swr t2,0x0(t4)
80105c68:  swl t3,0x7(t4)
80105c6c:  swr t3,0x4(t4)
80105c70:  lwl t2,0xb(t5)
80105c74:  lwr t2,0x8(t5)
80105c78:  nop
80105c7c:  swl t2,0xb(t4)
80105c80:  swr t2,0x8(t4)
80105c84:  sb a0,0x59d2(v0)
80105c88:  jal 0x8004f580
80105c8c:  _clear a0
80105c90:  lw ra,0x30(sp)
80105c94:  lw s1,0x2c(sp)
80105c98:  lw s0,0x28(sp)
80105c9c:  jr ra
80105ca0:  _addiu sp,sp,0x38
