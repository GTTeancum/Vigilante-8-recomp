# addr: 0x80017ba8  name: FUN_80017ba8
80017ba8:  addiu sp,sp,-0x38
80017bac:  sw s2,0x28(sp)
80017bb0:  move s2,a0
80017bb4:  sw s3,0x2c(sp)
80017bb8:  move s3,a1
80017bbc:  addiu a0,s2,0x80
80017bc0:  sw s0,0x20(sp)
80017bc4:  addiu s0,s3,0x20
80017bc8:  move a1,s0
80017bcc:  sw ra,0x34(sp)
80017bd0:  sw s4,0x30(sp)
80017bd4:  jal 0x80017240
80017bd8:  _sw s1,0x24(sp)
80017bdc:  srl v0,v0,0xf
80017be0:  sll a0,v1,0x11
80017be4:  or v0,v0,a0
80017be8:  sra v1,v1,0xf
80017bec:  move s1,v0
80017bf0:  bgez s1,0x80017d3c
80017bf4:  _addiu a0,s2,0x10
80017bf8:  move a1,s0
80017bfc:  jal 0x800434d0
80017c00:  _addiu a2,sp,0x10
80017c04:  lw a1,0x30(s3)
80017c08:  sll a0,s1,0x1
80017c0c:  move v0,a0
80017c10:  sra v1,a0,0x1f
80017c14:  lw a0,0x10(sp)
80017c18:  subu a1,zero,a1
80017c1c:  move t0,a1
80017c20:  sra t1,a1,0x1f
80017c24:  move t4,a0
80017c28:  sra t5,a0,0x1f
80017c2c:  sltu a0,t0,v0
80017c30:  subu t0,t0,v0
80017c34:  subu t1,t1,v1
80017c38:  subu t1,t1,a0
80017c3c:  multu t4,t0
80017c40:  addiu a1,sp,0x10
80017c44:  lw v0,0x4(a1)
80017c48:  mfhi s1
80017c4c:  mflo s0
80017c50:  move a2,v0
80017c54:  sra a3,v0,0x1f
80017c58:  multu a2,t0
80017c5c:  lw a0,0x8(a1)
80017c60:  mfhi t7
80017c64:  mflo t6
80017c68:  move v0,a0
80017c6c:  sra v1,a0,0x1f
80017c70:  multu v0,t0
80017c74:  mfhi t3
80017c78:  mflo t2
80017c7c:  nop
80017c80:  nop
80017c84:  mult t4,t1
80017c88:  mflo s4
80017c8c:  nop
80017c90:  nop
80017c94:  mult t0,t5
80017c98:  mflo t4
80017c9c:  nop
80017ca0:  nop
80017ca4:  mult a2,t1
80017ca8:  mflo t5
80017cac:  nop
80017cb0:  nop
80017cb4:  mult t0,a3
80017cb8:  mflo a3
80017cbc:  nop
80017cc0:  nop
80017cc4:  mult v0,t1
80017cc8:  mflo t8
80017ccc:  nop
80017cd0:  nop
80017cd4:  mult t0,v1
80017cd8:  move a0,s2
80017cdc:  addu s1,s1,s4
80017ce0:  addu s1,s1,t4
80017ce4:  addiu a2,s3,0x14
80017ce8:  addu t7,t7,t5
80017cec:  addu t7,t7,a3
80017cf0:  addu t3,t3,t8
80017cf4:  mflo v0
80017cf8:  addu t3,t3,v0
80017cfc:  srl s0,s0,0xc
80017d00:  sll v0,s1,0x14
80017d04:  or s0,s0,v0
80017d08:  sra s1,s1,0xc
80017d0c:  srl t6,t6,0xc
80017d10:  sll v0,t7,0x14
80017d14:  or t6,t6,v0
80017d18:  sra t7,t7,0xc
80017d1c:  srl t2,t2,0xc
80017d20:  sll v0,t3,0x14
80017d24:  or t2,t2,v0
80017d28:  sra t3,t3,0xc
80017d2c:  sw s0,0x10(sp)
80017d30:  sw t6,0x4(a1)
80017d34:  jal 0x80017594
80017d38:  _sw t2,0x8(a1)
80017d3c:  lw ra,0x34(sp)
80017d40:  lw s4,0x30(sp)
80017d44:  lw s3,0x2c(sp)
80017d48:  lw s2,0x28(sp)
80017d4c:  lw s1,0x24(sp)
80017d50:  lw s0,0x20(sp)
80017d54:  jr ra
80017d58:  _addiu sp,sp,0x38
