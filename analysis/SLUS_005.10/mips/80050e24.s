# addr: 0x80050e24  name: SYS_OBJ_1C20
80050e24:  addiu sp,sp,-0x30
80050e28:  sw s1,0x14(sp)
80050e2c:  move s1,a0
80050e30:  sw s2,0x18(sp)
80050e34:  move s2,a1
80050e38:  sw ra,0x28(sp)
80050e3c:  sw s5,0x24(sp)
80050e40:  sw s4,0x20(sp)
80050e44:  sw s3,0x1c(sp)
80050e48:  jal 0x80051b90
80050e4c:  _sw s0,0x10(sp)
80050e50:  lh a1,0x4(s1)
80050e54:  lhu v1,0x4(s1)
80050e58:  bltz a1,0x80050e88
80050e5c:  _clear s5
80050e60:  move a0,v1
80050e64:  lui v0,0x8006
80050e68:  lh v0,0x5028(v0)
80050e6c:  lui v1,0x8006
80050e70:  lhu v1,0x5028(v1)
80050e74:  slt v0,v0,a1
80050e78:  beq v0,zero,0x80050e8c
80050e7c:  _nop
80050e80:  j 0x80050e8c
80050e84:  _move a0,v1
80050e88:  clear a0
