# addr: 0x800434f8  name: FUN_800434f8
800434f8:  move t7,ra
800434fc:  gte_ldVXY0 0x0(a1)
80043500:  jal 0x8004366c
80043504:  _gte_ldVZ0 0x4(a1)
80043508:  nop
8004350c:  RTV0
80043510:  gte_stIR1 t0
80043514:  gte_stIR2 t1
80043518:  gte_stIR3 t2
8004351c:  sh t0,0x0(a2)
80043520:  sh t1,0x2(a2)
80043524:  jr t7
80043528:  _sh t2,0x4(a2)
