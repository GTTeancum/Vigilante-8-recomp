# addr: 0x8004cf04  name: CompMatrixLV
8004cf04:  lw t0,0x0(a0)
8004cf08:  lw t1,0x4(a0)
8004cf0c:  lw t2,0x8(a0)
8004cf10:  lw t3,0xc(a0)
8004cf14:  lw t4,0x10(a0)
8004cf18:  gte_ldR11R12 t0
8004cf1c:  gte_ldR13R21 t1
8004cf20:  gte_ldR22R23 t2
8004cf24:  gte_ldR31R32 t3
8004cf28:  gte_ldR33 t4
8004cf2c:  lhu t0,0x0(a1)
8004cf30:  lw t1,0x4(a1)
8004cf34:  lw t2,0xc(a1)
8004cf38:  lui at,0xffff
8004cf3c:  and t1,t1,at
8004cf40:  or t0,t0,t1
8004cf44:  gte_ldVXY0 t0
8004cf48:  gte_ldVZ0 t2
8004cf4c:  nop
8004cf50:  RTV0
8004cf54:  lhu t0,0x2(a1)
8004cf58:  lw t1,0x8(a1)
8004cf5c:  lh t2,0xe(a1)
8004cf60:  sll t1,t1,0x10
8004cf64:  or t0,t0,t1
8004cf68:  gte_stIR1 t3
8004cf6c:  gte_stIR2 t4
8004cf70:  gte_stIR3 t5
8004cf74:  gte_ldVXY0 t0
8004cf78:  gte_ldVZ0 t2
8004cf7c:  nop
8004cf80:  RTV0
8004cf84:  lhu t0,0x4(a1)
8004cf88:  lw t1,0x8(a1)
8004cf8c:  lw t2,0x10(a1)
8004cf90:  lui at,0xffff
8004cf94:  and t1,t1,at
8004cf98:  or t0,t0,t1
8004cf9c:  gte_stIR1 t6
8004cfa0:  gte_stIR2 t7
8004cfa4:  gte_stIR3 t8
8004cfa8:  gte_ldVXY0 t0
8004cfac:  gte_ldVZ0 t2
8004cfb0:  nop
8004cfb4:  RTV0
8004cfb8:  andi t3,t3,0xffff
8004cfbc:  sll t6,t6,0x10
8004cfc0:  or t6,t6,t3
8004cfc4:  sw t6,0x0(a2)
8004cfc8:  andi t5,t5,0xffff
8004cfcc:  sll t8,t8,0x10
8004cfd0:  or t8,t8,t5
8004cfd4:  sw t8,0xc(a2)
8004cfd8:  gte_stIR1 t0
8004cfdc:  gte_stIR2 t1
8004cfe0:  gte_stIR3 0x10(a2)
8004cfe4:  sll t4,t4,0x10
8004cfe8:  andi t0,t0,0xffff
8004cfec:  or t0,t0,t4
8004cff0:  sw t0,0x4(a2)
8004cff4:  andi t7,t7,0xffff
8004cff8:  sll t1,t1,0x10
8004cffc:  or t1,t1,t7
8004d000:  sw t1,0x8(a2)
8004d004:  lw t0,0x14(a1)
8004d008:  lw t1,0x18(a1)
8004d00c:  lw t2,0x1c(a1)
8004d010:  bgez t0,0x8004d030
8004d014:  _nop
8004d018:  subu t0,zero,t0
8004d01c:  sra t3,t0,0xf
8004d020:  subu t3,zero,t3
8004d024:  andi t0,t0,0x7fff
8004d028:  b 0x8004d038
8004d02c:  _subu t0,zero,t0
8004d030:  sra t3,t0,0xf
8004d034:  andi t0,t0,0x7fff
8004d038:  bgez t1,0x8004d058
8004d03c:  _nop
8004d040:  subu t1,zero,t1
8004d044:  sra t4,t1,0xf
8004d048:  subu t4,zero,t4
8004d04c:  andi t1,t1,0x7fff
8004d050:  b 0x8004d060
8004d054:  _subu t1,zero,t1
8004d058:  sra t4,t1,0xf
8004d05c:  andi t1,t1,0x7fff
8004d060:  bgez t2,0x8004d080
8004d064:  _nop
8004d068:  subu t2,zero,t2
8004d06c:  sra t5,t2,0xf
8004d070:  subu t5,zero,t5
8004d074:  andi t2,t2,0x7fff
8004d078:  b 0x8004d088
8004d07c:  _subu t2,zero,t2
8004d080:  sra t5,t2,0xf
8004d084:  andi t2,t2,0x7fff
8004d088:  ldsv_ t3,t4,t5
8004d094:  nop
8004d098:  RTIR_SF0
8004d09c:  read_mt t3,t4,t5
8004d0a8:  ldsv_ t0,t1,t2
8004d0b4:  nop
8004d0b8:  RTIR
8004d0bc:  bgez t3,0x8004d0d4
8004d0c0:  _nop
8004d0c4:  subu t3,zero,t3
8004d0c8:  sll t3,t3,0x3
8004d0cc:  b 0x8004d0d8
8004d0d0:  _subu t3,zero,t3
8004d0d4:  sll t3,t3,0x3
8004d0d8:  bgez t4,0x8004d0f0
8004d0dc:  _nop
8004d0e0:  subu t4,zero,t4
8004d0e4:  sll t4,t4,0x3
8004d0e8:  b 0x8004d0f4
8004d0ec:  _subu t4,zero,t4
8004d0f0:  sll t4,t4,0x3
8004d0f4:  bgez t5,0x8004d10c
8004d0f8:  _nop
8004d0fc:  subu t5,zero,t5
8004d100:  sll t5,t5,0x3
8004d104:  b 0x8004d110
8004d108:  _subu t5,zero,t5
8004d10c:  sll t5,t5,0x3
8004d110:  read_mt t0,t1,t2
8004d11c:  addu t0,t0,t3
8004d120:  addu t1,t1,t4
8004d124:  addu t2,t2,t5
8004d128:  lw t3,0x14(a0)
8004d12c:  lw t4,0x18(a0)
8004d130:  lw t5,0x1c(a0)
8004d134:  addu t0,t0,t3
8004d138:  addu t1,t1,t4
8004d13c:  addu t2,t2,t5
8004d140:  sw t0,0x14(a2)
8004d144:  sw t1,0x18(a2)
8004d148:  sw t2,0x1c(a2)
8004d14c:  jr ra
8004d150:  _move v0,a2
