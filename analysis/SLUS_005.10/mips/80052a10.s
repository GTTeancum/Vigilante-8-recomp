# addr: 0x80052a10  name: PRNT_OBJ_3CC
80052a10:  bltz s4,0x80052a20
80052a14:  _sw s4,0x3c(sp)
80052a18:  li v0,-0x21
80052a1c:  and s3,s3,v0
80052a20:  bne v1,zero,0x80052a38
80052a24:  _addiu s2,sp,0x38
80052a28:  lw a3,0x3c(sp)
80052a2c:  nop
80052a30:  beq a3,zero,0x80052aac
80052a34:  _subu v0,sp,s2
80052a38:  divu v1,a1
80052a3c:  bne a1,zero,0x80052a48
80052a40:  _nop
80052a44:  break 0x1c00
80052a48:  mflo v1
80052a4c:  mfhi v0
80052a50:  lw a3,0x48(sp)
80052a54:  nop
80052a58:  addu v0,a3,v0
80052a5c:  lbu a0,0x0(v0)
80052a60:  addiu s2,s2,-0x1
80052a64:  bne v1,zero,0x80052a38
80052a68:  _sb a0,0x0(s2)
80052a6c:  lui a3,0x8001
80052a70:  addiu a3,a3,0x1384
80052a74:  andi v0,s3,0x8
80052a78:  beq v0,zero,0x80052aa8
80052a7c:  _sw a3,0x48(sp)
80052a80:  li v0,0x8
80052a84:  bne a1,v0,0x80052aac
80052a88:  _subu v0,sp,s2
80052a8c:  sll v0,a0,0x18
80052a90:  sra v0,v0,0x18
80052a94:  li v1,0x30
80052a98:  beq v0,v1,0x80052aac
80052a9c:  _subu v0,sp,s2
80052aa0:  addiu s2,s2,-0x1
80052aa4:  sb v1,0x0(s2)
80052aa8:  subu v0,sp,s2
80052aac:  addiu s6,v0,0x38
