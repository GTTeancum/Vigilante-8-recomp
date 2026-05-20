# addr: 0x80052984  name: PRNT_OBJ_340
80052984:  andi v0,s3,0x1
80052988:  bne v0,zero,0x800529a8
8005298c:  _andi v0,s3,0x4
80052990:  beq v0,zero,0x800529a8
80052994:  _li a1,0xa
80052998:  addiu s1,s1,0x4
8005299c:  lh v1,-0x4(s1)
800529a0:  j 0x80052a10
800529a4:  _clear s8
800529a8:  addiu s1,s1,0x4
800529ac:  lw v1,-0x4(s1)
800529b0:  j 0x80052a0c
800529b4:  _li a1,0xa
