# addr: 0x8004d914  name: RotMatrixY
8004d914:  move t7,a0
8004d918:  move v0,a1
8004d91c:  bgez t7,0x8004d954
8004d920:  _andi t9,t7,0xfff
8004d924:  subu t7,zero,t7
8004d928:  bgez t7,0x8004d930
8004d92c:  _andi t7,t7,0xfff
8004d930:  sll t8,t7,0x2
8004d934:  lui t9,0x8006
8004d938:  addu t9,t9,t8
8004d93c:  lw t9,0x7b4(t9)
8004d940:  nop
8004d944:  sll t6,t9,0x10
8004d948:  sra t1,t6,0x10
8004d94c:  j 0x8004d978
8004d950:  _sra t0,t9,0x10
8004d954:  sll t8,t9,0x2
8004d958:  lui t9,0x8006
8004d95c:  addu t9,t9,t8
8004d960:  lw t9,0x7b4(t9)
8004d964:  nop
8004d968:  sll t8,t9,0x10
8004d96c:  sra t7,t8,0x10
8004d970:  subu t1,zero,t7
8004d974:  sra t0,t9,0x10
