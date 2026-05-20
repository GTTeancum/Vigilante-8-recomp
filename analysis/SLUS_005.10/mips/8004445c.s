# addr: 0x8004445c  name: FUN_8004445c
8004445c:  lh a3,0x8e4(gp)
80044460:  addiu sp,sp,-0x18
80044464:  sw ra,0x10(sp)
80044468:  sll v0,a3,0x10
8004446c:  jal 0x800443c8
80044470:  _addu a3,v0,a3
80044474:  lw ra,0x10(sp)
80044478:  nop
8004447c:  jr ra
80044480:  _addiu sp,sp,0x18
