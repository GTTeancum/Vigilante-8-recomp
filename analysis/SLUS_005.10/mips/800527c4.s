# addr: 0x800527c4  name: PRNT_OBJ_180
800527c4:  clear s0
800527c8:  lbu v0,0x0(s7)
800527cc:  addiu s7,s7,0x1
800527d0:  sll v1,s0,0x2
800527d4:  addu v1,v1,s0
800527d8:  sll v1,v1,0x1
800527dc:  addiu v1,v1,-0x30
800527e0:  lbu a0,0x0(s7)
800527e4:  nop
800527e8:  sltiu a0,a0,0x80
800527ec:  beq a0,zero,0x80052814
800527f0:  _addu s0,v1,v0
800527f4:  lbu v0,0x0(s7)
800527f8:  lui at,0x8006
800527fc:  addu at,at,v0
80052800:  lbu v0,0x5175(at)
80052804:  nop
80052808:  andi v0,v0,0x4
8005280c:  bne v0,zero,0x800527c8
80052810:  _nop
80052814:  sw s0,0x44(sp)
80052818:  j 0x800526c0
8005281c:  _addiu s7,s7,-0x1
