# addr: 0x8001db54  name: FUN_8001db54
8001db54:  move t1,a1
8001db58:  lui t2,0x8007
8001db5c:  addiu t2,t2,-0x880
8001db60:  SetRotMatrix t2
8001db88:  lw a2,0x0(a0)
8001db8c:  lui v0,0x8007
8001db90:  addiu v0,v0,-0x920
8001db94:  lw a3,0x14(v0)
8001db98:  lw a1,0x4(a0)
8001db9c:  lw t0,0x18(v0)
8001dba0:  lw v1,0x8(a0)
8001dba4:  lw v0,0x1c(v0)
8001dba8:  subu a2,a2,a3
8001dbac:  sra a2,a2,0x8
8001dbb0:  subu a1,a1,t0
8001dbb4:  sra a1,a1,0x8
8001dbb8:  subu v1,v1,v0
8001dbbc:  sra v1,v1,0x8
8001dbc0:  ldsv_ a2,a1,v1
8001dbcc:  nRTIR
8001dbd8:  sra t1,t1,0x8
8001dbdc:  clear v1
8001dbe0:  gte_stIR1 v0
8001dbe4:  nop
8001dbe8:  slt v0,v0,t1
8001dbec:  beq v0,zero,0x8001dc14
8001dbf0:  _nop
8001dbf4:  gte_stIR2 v0
8001dbf8:  nop
8001dbfc:  slt v0,v0,t1
8001dc00:  beq v0,zero,0x8001dc14
8001dc04:  _nop
8001dc08:  gte_stIR3 v0
8001dc0c:  nop
8001dc10:  slt v1,v0,t1
8001dc14:  jr ra
8001dc18:  _move v0,v1
