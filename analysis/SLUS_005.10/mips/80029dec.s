# addr: 0x80029dec  name: FUN_80029dec
80029dec:  clear a3
80029df0:  li t1,0x2
80029df4:  li t0,0x68
80029df8:  li a2,0xff
80029dfc:  lui v0,0x800a
80029e00:  addiu a1,v0,0x1e24
80029e04:  clear a0
80029e08:  move v1,a1
80029e0c:  sb t1,0x3(v1)
80029e10:  sb t0,0x7(v1)
80029e14:  sb a2,0x4(v1)
80029e18:  sb a2,0x5(v1)
80029e1c:  sb a2,0x6(v1)
80029e20:  addiu a0,a0,0x1
80029e24:  slti v0,a0,0x10
80029e28:  bne v0,zero,0x80029e0c
80029e2c:  _addiu v1,v1,0x14
80029e30:  addiu a3,a3,0x1
80029e34:  slti v0,a3,0x2
80029e38:  bne v0,zero,0x80029e04
80029e3c:  _addiu a1,a1,0x504
80029e40:  jr ra
80029e44:  _nop
