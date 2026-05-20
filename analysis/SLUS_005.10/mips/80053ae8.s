# addr: 0x80053ae8  name: COUNTER_OBJ_74
80053ae8:  beq v0,zero,0x80053af4
80053aec:  _li v0,0x1
80053af0:  ori a3,a3,0x10
80053af4:  lui a0,0x8006
80053af8:  lw a0,0x5238(a0)
80053afc:  sll v1,t0,0x4
80053b00:  addu v1,v1,a0
80053b04:  sh a3,0x4(v1)
