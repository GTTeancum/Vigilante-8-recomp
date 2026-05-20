# addr: 0x801117cc  name: FUN_801117cc
801117cc:  addiu sp,sp,-0x28
801117d0:  sw s0,0x10(sp)
801117d4:  move s0,a0
801117d8:  sw s1,0x14(sp)
801117dc:  move s1,a1
801117e0:  sw s2,0x18(sp)
801117e4:  move s2,a2
801117e8:  sw s3,0x1c(sp)
801117ec:  sw ra,0x20(sp)
801117f0:  sltiu a0,zero,0x46c0
801117f4:  move s3,a3
801117f8:  jal 0x80053a24
801117fc:  _nop
80111800:  sltiu a0,zero,0x46a2
80111804:  nop
80111808:  jal 0x80053a34
8011180c:  _nop
80111810:  jal 0x80053a54
80111814:  _clear a0
80111818:  sltiu a0,zero,0x4631
8011181c:  nop
80111820:  move a0,s0
80111824:  move a1,s1
80111828:  move a2,s2
8011182c:  sltiu a0,zero,0x4688
80111830:  move a3,s3
80111834:  li v0,0x1
80111838:  lui at,0x8011
8011183c:  sw v0,0x3374(at)
80111840:  lw ra,0x20(sp)
80111844:  lw s3,0x1c(sp)
80111848:  lw s2,0x18(sp)
8011184c:  lw s1,0x14(sp)
80111850:  lw s0,0x10(sp)
80111854:  jr ra
80111858:  _addiu sp,sp,0x28
