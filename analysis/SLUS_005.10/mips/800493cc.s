# addr: 0x800493cc  name: CdGetSector2
800493cc:  addiu sp,sp,-0x18
800493d0:  sw ra,0x10(sp)
800493d4:  jal 0x8004ab44
800493d8:  _nop
800493dc:  lw ra,0x10(sp)
800493e0:  sltiu v0,v0,0x1
800493e4:  jr ra
800493e8:  _addiu sp,sp,0x18
