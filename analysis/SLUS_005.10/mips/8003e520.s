# addr: 0x8003e520  name: FUN_8003e520
8003e520:  addiu sp,sp,-0x38
8003e524:  sw s0,0x30(sp)
8003e528:  move s0,a0
8003e52c:  lui a0,0x8007
8003e530:  addiu a0,a0,-0x980
8003e534:  addiu a1,s0,0x4
8003e538:  sw ra,0x34(sp)
8003e53c:  jal 0x8004cf04
8003e540:  _addiu a2,sp,0x10
8003e544:  lw a0,0x0(s0)
8003e548:  lw a2,0x60c(gp)
8003e54c:  jal 0x8001be5c
8003e550:  _addiu a1,sp,0x10
8003e554:  lw ra,0x34(sp)
8003e558:  lw s0,0x30(sp)
8003e55c:  jr ra
8003e560:  _addiu sp,sp,0x38
