# addr: 0x80050bf4  name: SYS_OBJ_19F0
80050bf4:  addiu sp,sp,-0x20
80050bf8:  move t0,a0
80050bfc:  sw ra,0x1c(sp)
80050c00:  sw s2,0x18(sp)
80050c04:  sw s1,0x14(sp)
80050c08:  sw s0,0x10(sp)
80050c0c:  lh v1,0x4(t0)
80050c10:  lhu a0,0x4(t0)
80050c14:  bltz v1,0x80050c48
80050c18:  _move t1,a1
80050c1c:  lui v0,0x8006
80050c20:  lh v0,0x5028(v0)
80050c24:  nop
80050c28:  addiu v0,v0,-0x1
80050c2c:  slt v0,v0,v1
80050c30:  lui v1,0x8006
80050c34:  lhu v1,0x5028(v1)
80050c38:  bne v0,zero,0x80050c4c
80050c3c:  _addiu v0,v1,-0x1
80050c40:  j 0x80050c4c
80050c44:  _move v0,a0
80050c48:  clear v0
