# addr: 0x800493ac  name: CdGetSector
800493ac:  addiu sp,sp,-0x18
800493b0:  sw ra,0x10(sp)
800493b4:  jal 0x8004aa44
800493b8:  _nop
800493bc:  lw ra,0x10(sp)
800493c0:  sltiu v0,v0,0x1
800493c4:  jr ra
800493c8:  _addiu sp,sp,0x18
