# addr: 0x80052254  name: GetClut
80052254:  sll v0,a1,0x6
80052258:  sra a0,a0,0x4
8005225c:  andi a0,a0,0x3f
80052260:  or v0,v0,a0
80052264:  jr ra
80052268:  _andi v0,v0,0xffff
