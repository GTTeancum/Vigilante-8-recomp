# addr: 0x8001e9a0  name: FUN_8001e9a0
8001e9a0:  addiu sp,sp,-0x38
8001e9a4:  sw ra,0x34(sp)
8001e9a8:  sw s8,0x30(sp)
8001e9ac:  sw s7,0x2c(sp)
8001e9b0:  sw s6,0x28(sp)
8001e9b4:  sw s5,0x24(sp)
8001e9b8:  sw s4,0x20(sp)
8001e9bc:  sw s3,0x1c(sp)
8001e9c0:  sw s2,0x18(sp)
8001e9c4:  sw s1,0x14(sp)
8001e9c8:  sw s0,0x10(sp)
8001e9cc:  sw a0,0x38(sp)
8001e9d0:  sw a1,0x3c(sp)
8001e9d4:  lw s3,0x5c(a0)
8001e9d8:  move s6,a2
8001e9dc:  beq s3,zero,0x8001ebe8
8001e9e0:  _move s7,a3
8001e9e4:  lw v0,0x5c(a1)
8001e9e8:  nop
8001e9ec:  beq v0,zero,0x8001ec18
8001e9f0:  _clear v0
8001e9f4:  lhu v0,0x0(s3)
8001e9f8:  nop
8001e9fc:  beq v0,zero,0x8001ebe8
8001ea00:  _li s8,0x2
8001ea04:  lw t0,0x3c(sp)
8001ea08:  lhu v0,0x0(s3)
8001ea0c:  lw s1,0x5c(t0)
8001ea10:  li t0,0x1
8001ea14:  beq v0,t0,0x8001ea2c
8001ea18:  _nop
8001ea1c:  beq v0,s8,0x8001ebac
8001ea20:  _nop
8001ea24:  j 0x8001ebdc
8001ea28:  _nop
8001ea2c:  lhu v0,0x0(s1)
8001ea30:  nop
8001ea34:  beq v0,zero,0x8001eb14
8001ea38:  _move s5,s3
8001ea3c:  addiu s4,s3,0x4
8001ea40:  lhu v0,0x0(s1)
8001ea44:  li t0,0x1
8001ea48:  beq v0,t0,0x8001ea60
8001ea4c:  _move a0,s4
8001ea50:  beq v0,s8,0x8001ea9c
8001ea54:  _nop
8001ea58:  j 0x8001eb08
8001ea5c:  _nop
8001ea60:  move a1,s6
8001ea64:  addiu s0,s1,0x4
8001ea68:  move a2,s0
8001ea6c:  jal 0x8001e1c0
8001ea70:  _move a3,s7
8001ea74:  beq v0,zero,0x8001ea94
8001ea78:  _move a0,s0
8001ea7c:  move a1,s7
8001ea80:  move a2,s4
8001ea84:  jal 0x8001e1c0
8001ea88:  _move a3,s6
8001ea8c:  bne v0,zero,0x8001ebf4
8001ea90:  _lui v0,0x1f80
8001ea94:  j 0x8001eb04
8001ea98:  _addiu s1,s1,0x1c
8001ea9c:  lhu v0,0x2(s1)
8001eaa0:  nop
8001eaa4:  beq v0,zero,0x8001ebf0
8001eaa8:  _clear s0
8001eaac:  li s2,0x4
8001eab0:  addiu a0,s5,0x4
8001eab4:  move a1,s6
8001eab8:  addu a2,s1,s2
8001eabc:  jal 0x8001e408
8001eac0:  _move a3,s7
8001eac4:  beq v0,zero,0x8001eae8
8001eac8:  _addiu s0,s0,0x1
8001eacc:  lhu v0,0x2(s1)
8001ead0:  nop
8001ead4:  slt v0,s0,v0
8001ead8:  bne v0,zero,0x8001eab0
8001eadc:  _addiu s2,s2,0xc
8001eae0:  j 0x8001ebf4
8001eae4:  _lui v0,0x1f80
8001eae8:  lhu v1,0x2(s1)
8001eaec:  nop
8001eaf0:  sll v0,v1,0x1
8001eaf4:  addu v0,v0,v1
8001eaf8:  sll v0,v0,0x2
8001eafc:  addiu v0,v0,0x4
8001eb00:  addu s1,s1,v0
8001eb04:  lhu v0,0x0(s1)
8001eb08:  nop
8001eb0c:  bne v0,zero,0x8001ea40
8001eb10:  _nop
8001eb14:  j 0x8001ebd8
8001eb18:  _addiu s3,s5,0x1c
8001eb1c:  lhu v0,0x0(s1)
8001eb20:  nop
8001eb24:  beq v0,t0,0x8001eb3c
8001eb28:  _nop
8001eb2c:  beq v0,s8,0x8001eb90
8001eb30:  _nop
8001eb34:  j 0x8001ebb0
8001eb38:  _nop
8001eb3c:  lhu v0,0x2(s3)
8001eb40:  nop
8001eb44:  beq v0,zero,0x8001ebf0
8001eb48:  _clear s0
8001eb4c:  li s2,0x4
8001eb50:  addiu a0,s1,0x4
8001eb54:  move a1,s7
8001eb58:  addu a2,s3,s2
8001eb5c:  jal 0x8001e408
8001eb60:  _move a3,s6
8001eb64:  beq v0,zero,0x8001eb88
8001eb68:  _addiu s0,s0,0x1
8001eb6c:  lhu v0,0x2(s3)
8001eb70:  nop
8001eb74:  slt v0,s0,v0
8001eb78:  bne v0,zero,0x8001eb50
8001eb7c:  _addiu s2,s2,0xc
8001eb80:  j 0x8001ebf4
8001eb84:  _lui v0,0x1f80
8001eb88:  j 0x8001ebac
8001eb8c:  _addiu s1,s1,0x1c
8001eb90:  lhu v1,0x2(s1)
8001eb94:  nop
8001eb98:  sll v0,v1,0x1
8001eb9c:  addu v0,v0,v1
8001eba0:  sll v0,v0,0x2
8001eba4:  addiu v0,v0,0x4
8001eba8:  addu s1,s1,v0
8001ebac:  lhu v0,0x0(s1)
8001ebb0:  nop
8001ebb4:  bne v0,zero,0x8001eb1c
8001ebb8:  _li t0,0x1
8001ebbc:  lhu v1,0x2(s3)
8001ebc0:  nop
8001ebc4:  sll v0,v1,0x1
8001ebc8:  addu v0,v0,v1
8001ebcc:  sll v0,v0,0x2
8001ebd0:  addiu v0,v0,0x4
8001ebd4:  addu s3,s3,v0
8001ebd8:  lhu v0,0x0(s3)
8001ebdc:  nop
8001ebe0:  bne v0,zero,0x8001ea04
8001ebe4:  _nop
8001ebe8:  j 0x8001ec18
8001ebec:  _clear v0
8001ebf0:  lui v0,0x1f80
8001ebf4:  addiu v0,v0,0x0
8001ebf8:  sw s3,0x4(v0)
8001ebfc:  sw s1,0x8(v0)
8001ec00:  lw t0,0x38(sp)
8001ec04:  nop
8001ec08:  sw t0,0xc(v0)
8001ec0c:  lw t0,0x3c(sp)
8001ec10:  nop
8001ec14:  sw t0,0x10(v0)
8001ec18:  lw ra,0x34(sp)
8001ec1c:  lw s8,0x30(sp)
8001ec20:  lw s7,0x2c(sp)
8001ec24:  lw s6,0x28(sp)
8001ec28:  lw s5,0x24(sp)
8001ec2c:  lw s4,0x20(sp)
8001ec30:  lw s3,0x1c(sp)
8001ec34:  lw s2,0x18(sp)
8001ec38:  lw s1,0x14(sp)
8001ec3c:  lw s0,0x10(sp)
8001ec40:  jr ra
8001ec44:  _addiu sp,sp,0x38
