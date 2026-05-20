# addr: 0x8004b040  name: CDREAD_OBJ_32C
8004b040:  addiu sp,sp,-0x28
8004b044:  sw s2,0x20(sp)
8004b048:  move s2,a0
8004b04c:  clear a0
8004b050:  sw ra,0x24(sp)
8004b054:  sw s1,0x1c(sp)
8004b058:  jal 0x80048fa8
8004b05c:  _sw s0,0x18(sp)
8004b060:  jal 0x80048fbc
8004b064:  _clear a0
8004b068:  lui s0,0x8006
8004b06c:  addiu s0,s0,0x3c8
8004b070:  lw v0,0x0(s0)
8004b074:  nop
8004b078:  andi v0,v0,0x1
8004b07c:  beq v0,zero,0x8004b08c
8004b080:  _nop
8004b084:  jal 0x800493ec
8004b088:  _clear a0
8004b08c:  jal 0x80048e24
8004b090:  _nop
8004b094:  andi v0,v0,0x10
8004b098:  beq v0,zero,0x8004b0f0
8004b09c:  _nop
8004b0a0:  jal 0x80047e44
8004b0a4:  _li a0,-0x1
8004b0a8:  andi v0,v0,0x3f
8004b0ac:  bne v0,zero,0x8004b0c4
8004b0b0:  _li a0,0x1
8004b0b4:  lui a0,0x8001
8004b0b8:  jal 0x80053884
8004b0bc:  _addiu a0,a0,0x10ec
8004b0c0:  li a0,0x1
8004b0c4:  jal 0x8004910c
8004b0c8:  _clear a1
8004b0cc:  jal 0x80047e44
8004b0d0:  _li a0,-0x1
8004b0d4:  addiu v1,s0,-0x30
8004b0d8:  sw v0,0x1c(v1)
8004b0dc:  li v0,-0x1
8004b0e0:  sw v0,0x14(v1)
8004b0e4:  lw v0,0x14(v1)
8004b0e8:  j 0x8004b218
8004b0ec:  _nop
8004b0f0:  beq s2,zero,0x8004b148
8004b0f4:  _nop
8004b0f8:  lui a0,0x8001
8004b0fc:  jal 0x80053884
8004b100:  _addiu a0,a0,0x1104
8004b104:  li a0,0x9
8004b108:  clear a1
8004b10c:  jal 0x80048fd0
8004b110:  _clear a2
8004b114:  jal 0x80048e54
8004b118:  _nop
8004b11c:  li a0,0x2
8004b120:  move a1,v0
8004b124:  jal 0x80048fd0
8004b128:  _clear a2
8004b12c:  bne v0,zero,0x8004b148
8004b130:  _addiu v0,s0,-0x30
8004b134:  li v1,-0x1
8004b138:  sw v1,0x14(v0)
8004b13c:  lw v0,-0x1c(s0)
8004b140:  j 0x8004b218
8004b144:  _nop
8004b148:  jal 0x80048ecc
8004b14c:  _nop
8004b150:  lui s1,0x8006
8004b154:  addiu s1,s1,0x3a4
8004b158:  lw s0,0x0(s1)
8004b15c:  nop
8004b160:  sb s0,0x10(sp)
8004b164:  jal 0x80048e34
8004b168:  _andi s0,s0,0xff
8004b16c:  bne s0,v0,0x8004b17c
8004b170:  _li a0,0xe
8004b174:  beq s2,zero,0x8004b1a4
8004b178:  _nop
8004b17c:  addiu a1,sp,0x10
8004b180:  jal 0x80048fd0
8004b184:  _clear a2
8004b188:  bne v0,zero,0x8004b1a4
8004b18c:  _addiu v0,s1,-0xc
8004b190:  li v1,-0x1
8004b194:  sw v1,0x14(v0)
8004b198:  lw v0,0x14(v0)
8004b19c:  j 0x8004b218
8004b1a0:  _nop
8004b1a4:  jal 0x80048e54
8004b1a8:  _nop
8004b1ac:  jal 0x80049534
8004b1b0:  _move a0,v0
8004b1b4:  lui a0,0x8005
8004b1b8:  addiu a0,a0,-0x52ec
8004b1bc:  lui s0,0x8006
8004b1c0:  addiu s0,s0,0x398
8004b1c4:  jal 0x80048fbc
8004b1c8:  _sw v0,0x20(s0)
8004b1cc:  lw v0,0x30(s0)
8004b1d0:  nop
8004b1d4:  andi v0,v0,0x1
8004b1d8:  beq v0,zero,0x8004b1f0
8004b1dc:  _li a0,0x6
8004b1e0:  lui a0,0x8005
8004b1e4:  jal 0x800493ec
8004b1e8:  _addiu a0,a0,-0x508c
8004b1ec:  li a0,0x6
8004b1f0:  lw v0,0x4(s0)
8004b1f4:  clear a1
8004b1f8:  jal 0x8004910c
8004b1fc:  _sw v0,0x8(s0)
8004b200:  lw v0,0x0(s0)
8004b204:  li a0,-0x1
8004b208:  jal 0x80047e44
8004b20c:  _sw v0,0x14(s0)
8004b210:  sw v0,0x18(s0)
8004b214:  lw v0,0x14(s0)
