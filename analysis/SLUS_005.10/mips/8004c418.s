# addr: 0x8004c418  name: C_011_OBJ_9C4
8004c418:  bne s1,v0,0x8004c45c
8004c41c:  _nop
8004c420:  lui v1,0x8006
8004c424:  lw v1,0x450(v1)
8004c428:  nop
8004c42c:  lbu a0,0x2(v1)
8004c430:  sllv v0,v0,s0
8004c434:  j 0x8004c474
8004c438:  _or v0,a0,v0
8004c45c:  lui v1,0x8006
8004c460:  lw v1,0x450(v1)
8004c464:  sllv v0,v0,s0
8004c468:  lbu a0,0x2(v1)
8004c46c:  nor v0,zero,v0
8004c470:  and v0,a0,v0
