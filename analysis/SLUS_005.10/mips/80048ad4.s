# addr: 0x80048ad4  name: CdGetToc
80048ad4:  addiu sp,sp,-0x18
80048ad8:  sw ra,0x10(sp)
80048adc:  move a1,a0
80048ae0:  jal 0x80048af8
80048ae4:  _li a0,0x1
80048ae8:  lw ra,0x10(sp)
80048aec:  addiu sp,sp,0x18
80048af0:  jr ra
80048af4:  _nop
