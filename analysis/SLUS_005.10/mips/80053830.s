# addr: 0x80053830  name: SPRINTF_OBJ_82C
80053830:  lw v1,0x24c(sp)
80053834:  nop
80053838:  addiu v0,v1,0x1
8005383c:  sw v0,0x24c(sp)
80053840:  lb a1,0x1(v1)
80053844:  nop
80053848:  bne a1,zero,0x80053064
8005384c:  _li v0,0x25
80053850:  addu v0,s3,s2
80053854:  sb zero,0x0(v0)
80053858:  move v0,s2
8005385c:  lw ra,0x244(sp)
80053860:  lw s6,0x240(sp)
80053864:  lw s5,0x23c(sp)
80053868:  lw s4,0x238(sp)
8005386c:  lw s3,0x234(sp)
80053870:  lw s2,0x230(sp)
80053874:  lw s1,0x22c(sp)
80053878:  lw s0,0x228(sp)
8005387c:  jr ra
80053880:  _addiu sp,sp,0x248
