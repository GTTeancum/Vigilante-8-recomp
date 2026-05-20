# addr: 0x8011173c  name: FUN_8011173c
8011173c:  addiu sp,sp,-0x28
80111740:  sw s0,0x10(sp)
80111744:  move s0,a0
80111748:  sw s1,0x14(sp)
8011174c:  move s1,a1
80111750:  sw s2,0x18(sp)
80111754:  move s2,a2
80111758:  sw s3,0x1c(sp)
8011175c:  sw ra,0x20(sp)
80111760:  sltiu a0,zero,0x46c0
80111764:  move s3,a3
80111768:  jal 0x80053a24
8011176c:  _nop
80111770:  sltiu a0,zero,0x46a2
80111774:  nop
80111778:  jal 0x80053a34
8011177c:  _nop
80111780:  jal 0x80053a54
80111784:  _clear a0
80111788:  sltiu a0,zero,0x4631
8011178c:  nop
80111790:  move a0,s0
80111794:  move a1,s1
80111798:  move a2,s2
8011179c:  sltiu a0,zero,0x4694
801117a0:  move a3,s3
801117a4:  li v0,0x1
801117a8:  lui at,0x8011
801117ac:  sw v0,0x3374(at)
801117b0:  lw ra,0x20(sp)
801117b4:  lw s3,0x1c(sp)
801117b8:  lw s2,0x18(sp)
801117bc:  lw s1,0x14(sp)
801117c0:  lw s0,0x10(sp)
801117c4:  jr ra
801117c8:  _addiu sp,sp,0x28
