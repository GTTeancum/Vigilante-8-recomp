# addr: 0x8004ccb4  name: MulMatrix0
8004ccb4:  lw t0,0x0(a0)
8004ccb8:  lw t1,0x4(a0)
8004ccbc:  lw t2,0x8(a0)
8004ccc0:  lw t3,0xc(a0)
8004ccc4:  lw t4,0x10(a0)
8004ccc8:  gte_ldR11R12 t0
8004cccc:  gte_ldR13R21 t1
8004ccd0:  gte_ldR22R23 t2
8004ccd4:  gte_ldR31R32 t3
8004ccd8:  gte_ldR33 t4
8004ccdc:  lhu t0,0x0(a1)
8004cce0:  lw t1,0x4(a1)
8004cce4:  lw t2,0xc(a1)
8004cce8:  lui at,0xffff
8004ccec:  and t1,t1,at
8004ccf0:  or t0,t0,t1
8004ccf4:  gte_ldVXY0 t0
8004ccf8:  gte_ldVZ0 t2
8004ccfc:  nop
8004cd00:  RTV0
8004cd04:  lhu t0,0x2(a1)
8004cd08:  lw t1,0x8(a1)
8004cd0c:  lh t2,0xe(a1)
8004cd10:  sll t1,t1,0x10
8004cd14:  or t0,t0,t1
8004cd18:  gte_stIR1 t3
8004cd1c:  gte_stIR2 t4
8004cd20:  gte_stIR3 t5
8004cd24:  gte_ldVXY0 t0
8004cd28:  gte_ldVZ0 t2
8004cd2c:  nop
8004cd30:  RTV0
8004cd34:  lhu t0,0x4(a1)
8004cd38:  lw t1,0x8(a1)
8004cd3c:  lw t2,0x10(a1)
8004cd40:  lui at,0xffff
8004cd44:  and t1,t1,at
8004cd48:  or t0,t0,t1
8004cd4c:  gte_stIR1 t6
8004cd50:  gte_stIR2 t7
8004cd54:  gte_stIR3 t8
8004cd58:  gte_ldVXY0 t0
8004cd5c:  gte_ldVZ0 t2
8004cd60:  nop
8004cd64:  RTV0
8004cd68:  andi t3,t3,0xffff
8004cd6c:  sll t6,t6,0x10
8004cd70:  or t6,t6,t3
8004cd74:  sw t6,0x0(a2)
8004cd78:  andi t5,t5,0xffff
8004cd7c:  sll t8,t8,0x10
8004cd80:  or t8,t8,t5
8004cd84:  sw t8,0xc(a2)
8004cd88:  gte_stIR1 t0
8004cd8c:  gte_stIR2 t1
8004cd90:  andi t0,t0,0xffff
8004cd94:  sll t4,t4,0x10
8004cd98:  or t0,t0,t4
8004cd9c:  sw t0,0x4(a2)
8004cda0:  andi t7,t7,0xffff
8004cda4:  sll t1,t1,0x10
8004cda8:  or t1,t1,t7
8004cdac:  sw t1,0x8(a2)
8004cdb0:  gte_stIR3 0x10(a2)
8004cdb4:  move v0,a2
8004cdb8:  jr ra
8004cdbc:  _nop
