# addr: 0x80100a9c  name: FUN_80100a9c
80100a9c:  addiu sp,sp,-0x20
80100aa0:  sw s1,0x14(sp)
80100aa4:  move s1,a0
80100aa8:  sw ra,0x18(sp)
80100aac:  bne a1,zero,0x80100bb8
80100ab0:  _sw s0,0x10(sp)
80100ab4:  lhu v0,0x80(s1)
80100ab8:  ori v1,zero,0xffff
80100abc:  addiu v0,v0,-0x1
80100ac0:  sh v0,0x80(s1)
80100ac4:  andi v0,v0,0xffff
80100ac8:  bne v0,v1,0x80100bb8
80100acc:  _li a2,0xa0
80100ad0:  lw a0,0x98(s1)
80100ad4:  lhu a1,0x96(s1)
80100ad8:  jal 0x8001ac44
80100adc:  _li a3,0x8
80100ae0:  jal 0x80017160
80100ae4:  _move s0,v0
80100ae8:  lw v1,0x0(s0)
80100aec:  andi v0,v0,0xfff
80100af0:  sll v0,v0,0x2
80100af4:  ori v1,v1,0x4b4
80100af8:  sw v1,0x0(s0)
80100afc:  lui v1,0x8006
80100b00:  addiu v1,v1,0x7b4
80100b04:  addu a0,v0,v1
80100b08:  lh v1,0x0(a0)
80100b0c:  lw v0,0x84(s1)
80100b10:  nop
80100b14:  mult v0,v1
80100b18:  mflo v0
80100b1c:  bgez v0,0x80100b28
80100b20:  _nop
80100b24:  addiu v0,v0,0xfff
80100b28:  sra v0,v0,0xc
80100b2c:  sw v0,0x88(s0)
80100b30:  lh v1,0x2(a0)
80100b34:  lw v0,0x84(s1)
80100b38:  nop
80100b3c:  mult v0,v1
80100b40:  mflo v0
80100b44:  bgez v0,0x80100b50
80100b48:  _nop
80100b4c:  addiu v0,v0,0xfff
80100b50:  sra v0,v0,0xc
80100b54:  jal 0x80017160
80100b58:  _sw v0,0x90(s0)
80100b5c:  lw v1,0x88(s1)
80100b60:  nop
80100b64:  mult v0,v1
80100b68:  move a0,s1
80100b6c:  mflo t0
80100b70:  sra v0,t0,0xf
80100b74:  addu v1,v1,v0
80100b78:  jal 0x8001d624
80100b7c:  _sw v1,0x8c(s0)
80100b80:  move a0,s0
80100b84:  lw t1,0x14(v0)
80100b88:  lw t2,0x18(v0)
80100b8c:  lw t3,0x1c(v0)
80100b90:  sw t1,0x24(a0)
80100b94:  sw t2,0x28(a0)
80100b98:  sw t3,0x2c(a0)
80100b9c:  lui v0,0x8004
80100ba0:  addiu v0,v0,0x4c4
80100ba4:  jal 0x800202f4
80100ba8:  _sw v0,0x64(a0)
80100bac:  lhu v0,0x82(s1)
80100bb0:  nop
80100bb4:  sh v0,0x80(s1)
80100bb8:  lw ra,0x18(sp)
80100bbc:  lw s1,0x14(sp)
80100bc0:  lw s0,0x10(sp)
80100bc4:  clear v0
80100bc8:  jr ra
80100bcc:  _addiu sp,sp,0x20
