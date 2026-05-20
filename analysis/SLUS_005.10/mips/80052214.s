# addr: 0x80052214  name: GetTPage
80052214:  andi v0,a0,0x3
80052218:  sll v0,v0,0x7
8005221c:  andi a1,a1,0x3
80052220:  sll a1,a1,0x5
80052224:  or v0,v0,a1
80052228:  andi v1,a3,0x100
8005222c:  sra v1,v1,0x4
80052230:  or v0,v0,v1
80052234:  andi a2,a2,0x3ff
80052238:  sra a2,a2,0x6
8005223c:  or v0,v0,a2
80052240:  andi a3,a3,0x200
80052244:  sll a3,a3,0x2
80052248:  jr ra
8005224c:  _or v0,v0,a3
