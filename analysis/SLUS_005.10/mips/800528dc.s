# addr: 0x800528dc  name: PRNT_OBJ_298
800528dc:  andi v0,s3,0x1
800528e0:  bne v0,zero,0x80052900
800528e4:  _andi v0,s3,0x4
800528e8:  beq v0,zero,0x80052900
800528ec:  _li a1,0x8
800528f0:  addiu s1,s1,0x4
800528f4:  lh v1,-0x4(s1)
800528f8:  j 0x80052a10
800528fc:  _clear s8
80052900:  addiu s1,s1,0x4
80052904:  lw v1,-0x4(s1)
80052908:  j 0x80052a0c
8005290c:  _li a1,0x8
