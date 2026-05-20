# addr: 0x8001178c  name: FUN_8001178c
8001178c:  addiu sp,sp,-0x20
80011790:  sw s1,0x14(sp)
80011794:  move s1,a0
80011798:  sw s2,0x18(sp)
8001179c:  move s2,a1
800117a0:  sw ra,0x1c(sp)
800117a4:  jal 0x800451c0
800117a8:  _sw s0,0x10(sp)
800117ac:  move s0,v0
800117b0:  bne s0,zero,0x8001181c
800117b4:  _move v0,s0
800117b8:  beq s1,zero,0x8001181c
800117bc:  _nop
800117c0:  jal 0x8004f580
800117c4:  _clear a0
800117c8:  lw v0,0x4(gp)
800117cc:  li a0,0x1
800117d0:  jal 0x80011914
800117d4:  _subu a0,a0,v0
800117d8:  jal 0x800165cc
800117dc:  _li a0,0x1
800117e0:  move a0,s1
800117e4:  jal 0x800451c0
800117e8:  _move a1,s2
800117ec:  move s0,v0
800117f0:  bne s0,zero,0x8001181c
800117f4:  _move v0,s0
800117f8:  jal 0x80020d3c
800117fc:  _nop
80011800:  bne v0,zero,0x800117e4
80011804:  _move a0,s1
80011808:  bne s0,zero,0x8001181c
8001180c:  _move v0,s0
80011810:  jal 0x800539a4
80011814:  _nop
80011818:  move v0,s0
8001181c:  lw ra,0x1c(sp)
80011820:  lw s2,0x18(sp)
80011824:  lw s1,0x14(sp)
80011828:  lw s0,0x10(sp)
8001182c:  jr ra
80011830:  _addiu sp,sp,0x20
