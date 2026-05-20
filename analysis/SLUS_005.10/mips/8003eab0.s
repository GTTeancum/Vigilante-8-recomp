# addr: 0x8003eab0  name: FUN_8003eab0
8003eab0:  addiu sp,sp,-0x38
8003eab4:  sw s0,0x20(sp)
8003eab8:  move s0,a0
8003eabc:  sw s3,0x2c(sp)
8003eac0:  move s3,a2
8003eac4:  sw ra,0x30(sp)
8003eac8:  sw s2,0x28(sp)
8003eacc:  beq a1,zero,0x8003eae8
8003ead0:  _sw s1,0x24(sp)
8003ead4:  li v0,0x3
8003ead8:  beq a1,v0,0x8003ebb4
8003eadc:  _clear v0
8003eae0:  j 0x8003ed1c
8003eae4:  _nop
8003eae8:  lw v0,0x24(s0)
8003eaec:  lw v1,0x88(s0)
8003eaf0:  addiu s1,s0,0x24
8003eaf4:  addu v0,v0,v1
8003eaf8:  sw v0,0x24(s0)
8003eafc:  lw v0,0x4(s1)
8003eb00:  addiu a1,s0,0x88
8003eb04:  lw a0,0x4(a1)
8003eb08:  lw v1,0x8(s1)
8003eb0c:  addu v0,v0,a0
8003eb10:  sw v0,0x4(s1)
8003eb14:  lw v0,0x8(a1)
8003eb18:  nop
8003eb1c:  addu v1,v1,v0
8003eb20:  sw v1,0x8(s1)
8003eb24:  lh a1,0x80(s0)
8003eb28:  lh a2,0x82(s0)
8003eb2c:  lh a3,0x84(s0)
8003eb30:  jal 0x800439b8
8003eb34:  _addiu a0,s0,0x10
8003eb38:  lw v0,0x8c(s0)
8003eb3c:  nop
8003eb40:  addiu v0,v0,0x5a
8003eb44:  blez v0,0x8003ed18
8003eb48:  _sw v0,0x8c(s0)
8003eb4c:  move a0,s0
8003eb50:  move a1,s1
8003eb54:  clear a2
8003eb58:  jal 0x8001d748
8003eb5c:  _move a3,a2
8003eb60:  lw v1,0x28(s0)
8003eb64:  move a1,v0
8003eb68:  slt v1,a1,v1
8003eb6c:  beq v1,zero,0x8003ed1c
8003eb70:  _clear v0
8003eb74:  lw v0,0x8c(s0)
8003eb78:  lbu a0,0x87(s0)
8003eb7c:  sw a1,0x28(s0)
8003eb80:  subu v0,zero,v0
8003eb84:  srl v1,v0,0x1f
8003eb88:  addu v0,v0,v1
8003eb8c:  sra v0,v0,0x1
8003eb90:  addiu a0,a0,-0x1
8003eb94:  sb a0,0x87(s0)
8003eb98:  andi a0,a0,0xff
8003eb9c:  bne a0,zero,0x8003ed18
8003eba0:  _sw v0,0x8c(s0)
8003eba4:  jal 0x800205f8
8003eba8:  _move a0,s0
8003ebac:  j 0x8003ed1c
8003ebb0:  _li v0,-0x1
8003ebb4:  lw s2,0x0(s3)
8003ebb8:  nop
8003ebbc:  lbu v1,0x4(s2)
8003ebc0:  li v0,0x2
8003ebc4:  bne v1,v0,0x8003ed1c
8003ebc8:  _clear v0
8003ebcc:  move a0,s0
8003ebd0:  jal 0x8001f5a0
8003ebd4:  _move a1,s3
8003ebd8:  lh v0,0x20(s3)
8003ebdc:  lw t0,0x88(s0)
8003ebe0:  nop
8003ebe4:  mult t0,v0
8003ebe8:  addiu a1,s3,0x20
8003ebec:  lh v1,0x2(a1)
8003ebf0:  addiu a0,s0,0x88
8003ebf4:  lw v0,0x4(a0)
8003ebf8:  mflo a3
8003ebfc:  nop
8003ec00:  nop
8003ec04:  mult v0,v1
8003ec08:  lh v1,0x4(a1)
8003ec0c:  lw v0,0x8(a0)
8003ec10:  mflo a2
8003ec14:  nop
8003ec18:  nop
8003ec1c:  mult v0,v1
8003ec20:  addu v0,a3,a2
8003ec24:  mflo v1
8003ec28:  addu v0,v0,v1
8003ec2c:  bgez v0,0x8003ec3c
8003ec30:  _sra s1,v0,0xb
8003ec34:  addiu v0,v0,0x7ff
8003ec38:  sra s1,v0,0xb
8003ec3c:  bgez s1,0x8003ed1c
8003ec40:  _clear v0
8003ec44:  sll v0,t0,0x7
8003ec48:  sw v0,0x10(sp)
8003ec4c:  lw v0,0x8c(s0)
8003ec50:  addiu a1,sp,0x10
8003ec54:  sll v0,v0,0x7
8003ec58:  sw v0,0x4(a1)
8003ec5c:  lw v0,0x90(s0)
8003ec60:  move a0,s2
8003ec64:  addiu a2,s0,0x24
8003ec68:  sll v0,v0,0x7
8003ec6c:  jal 0x800176f8
8003ec70:  _sw v0,0x8(a1)
8003ec74:  lh a0,0x6(s2)
8003ec78:  nop
8003ec7c:  bgez a0,0x8003ec94
8003ec80:  _nor a0,zero,a0
8003ec84:  li a1,0xff
8003ec88:  li a2,0x2
8003ec8c:  jal 0x80012068
8003ec90:  _li a3,0x80
8003ec94:  lh v0,0x20(s3)
8003ec98:  nop
8003ec9c:  mult s1,v0
8003eca0:  mflo v1
8003eca4:  bgez v1,0x8003ecb0
8003eca8:  _nop
8003ecac:  addiu v1,v1,0xfff
8003ecb0:  lw v0,0x88(s0)
8003ecb4:  sra v1,v1,0xc
8003ecb8:  subu v0,v0,v1
8003ecbc:  sw v0,0x88(s0)
8003ecc0:  lh v0,0x22(s3)
8003ecc4:  nop
8003ecc8:  mult s1,v0
8003eccc:  mflo v1
8003ecd0:  bgez v1,0x8003ecdc
8003ecd4:  _nop
8003ecd8:  addiu v1,v1,0xfff
8003ecdc:  lw v0,0x8c(s0)
8003ece0:  sra v1,v1,0xc
8003ece4:  subu v0,v0,v1
8003ece8:  sw v0,0x8c(s0)
8003ecec:  lh v0,0x24(s3)
8003ecf0:  nop
8003ecf4:  mult s1,v0
8003ecf8:  mflo v1
8003ecfc:  bgez v1,0x8003ed08
8003ed00:  _nop
8003ed04:  addiu v1,v1,0xfff
8003ed08:  lw v0,0x90(s0)
8003ed0c:  sra v1,v1,0xc
8003ed10:  subu v0,v0,v1
8003ed14:  sw v0,0x90(s0)
8003ed18:  clear v0
8003ed1c:  lw ra,0x30(sp)
8003ed20:  lw s3,0x2c(sp)
8003ed24:  lw s2,0x28(sp)
8003ed28:  lw s1,0x24(sp)
8003ed2c:  lw s0,0x20(sp)
8003ed30:  jr ra
8003ed34:  _addiu sp,sp,0x38
