# addr: 0x8004ff50  name: SYS_OBJ_D4C
8004ff50:  lbu v0,0x12(s1)
8004ff54:  lh v1,0x6(s1)
8004ff58:  bne v0,zero,0x8004ff64
8004ff5c:  _slti v0,v1,0x121
8004ff60:  slti v0,v1,0x101
8004ff64:  bne v0,zero,0x8004ff70
8004ff68:  _nop
8004ff6c:  ori s0,s0,0x24
8004ff70:  lui v0,0x8006
8004ff74:  lw v0,0x501c(v0)
8004ff78:  nop
8004ff7c:  lw v0,0x10(v0)
8004ff80:  nop
8004ff84:  jalr v0
8004ff88:  _move a0,s0
8004ff8c:  li v0,0x8
8004ff90:  sb v0,0x12(s1)
8004ff94:  lui a0,0x8006
8004ff98:  addiu a0,a0,0x5098
8004ff9c:  lhu v0,0x0(a0)
8004ffa0:  lh v1,0x8(s1)
8004ffa4:  sll v0,v0,0x10
8004ffa8:  sra v0,v0,0x10
8004ffac:  bne v0,v1,0x8005000c
8004ffb0:  _nop
8004ffb4:  lhu v0,0x2(a0)
8004ffb8:  lh v1,0xa(s1)
8004ffbc:  sll v0,v0,0x10
8004ffc0:  sra v0,v0,0x10
8004ffc4:  bne v0,v1,0x8005000c
8004ffc8:  _nop
8004ffcc:  lhu v0,0x4(a0)
8004ffd0:  lh v1,0xc(s1)
8004ffd4:  sll v0,v0,0x10
8004ffd8:  sra v0,v0,0x10
8004ffdc:  bne v0,v1,0x8005000c
8004ffe0:  _nop
8004ffe4:  lhu v0,0x6(a0)
8004ffe8:  lh v1,0xe(s1)
8004ffec:  sll v0,v0,0x10
8004fff0:  sra v0,v0,0x10
8004fff4:  bne v0,v1,0x8005000c
8004fff8:  _li v0,0x8
8004fffc:  lbu v1,0x12(s1)
80050000:  nop
80050004:  bne v1,v0,0x80050278
80050008:  _nop
8005000c:  jal 0x8004f1e8
80050010:  _nop
80050014:  sb v0,0x12(s1)
80050018:  andi v0,v0,0xff
8005001c:  lh a0,0xa(s1)
80050020:  bne v0,zero,0x8005002c
80050024:  _addiu s0,a0,0x13
80050028:  addiu s0,a0,0x10
8005002c:  lh v0,0xe(s1)
80050030:  nop
80050034:  bne v0,zero,0x80050040
80050038:  _addu s2,s0,v0
8005003c:  addiu s2,s0,0xf0
80050040:  lh v1,0x4(s1)
80050044:  nop
80050048:  slti v0,v1,0x119
8005004c:  bne v0,zero,0x8005007c
80050050:  _clear a2
80050054:  slti v0,v1,0x161
80050058:  bne v0,zero,0x8005007c
8005005c:  _li a2,0x1
80050060:  slti v0,v1,0x191
80050064:  bne v0,zero,0x8005007c
80050068:  _li a2,0x2
8005006c:  slti v0,v1,0x231
80050070:  beq v0,zero,0x8005007c
80050074:  _li a2,0x4
80050078:  li a2,0x3
8005007c:  lh v1,0x8(s1)
80050080:  lui v0,0x8006
80050084:  addu v0,v0,a2
80050088:  lbu v0,0x5120(v0)
8005008c:  nop
80050090:  mult v1,v0
80050094:  lbu v1,0x12(s1)
80050098:  nop
8005009c:  sll v0,v1,0x2
800500a0:  addu v0,v0,v1
800500a4:  addu v0,v0,a2
800500a8:  sll v0,v0,0x2
800500ac:  lui v1,0x8006
800500b0:  addu v1,v1,v0
800500b4:  lhu v1,0x50f8(v1)
800500b8:  lui at,0x8006
800500bc:  addu at,at,v0
800500c0:  lhu v0,0x50fa(at)
800500c4:  lh a1,0xc(s1)
800500c8:  subu v0,v0,v1
800500cc:  mflo a3
800500d0:  beq a1,zero,0x800500e4
800500d4:  _addu a0,v1,a3
800500d8:  mult v0,a1
800500dc:  mflo a3
800500e0:  sra v0,a3,0x8
800500e4:  addu v1,a0,v0
800500e8:  lbu v0,0x12(s1)
800500ec:  nop
800500f0:  beq v0,zero,0x80050184
800500f4:  _slti v0,a0,0x21c
800500f8:  bne v0,zero,0x80050110
800500fc:  _li a1,0x21c
80050100:  slti v0,a0,0xc95
80050104:  beq v0,zero,0x80050110
80050108:  _li a1,0xc94
8005010c:  move a1,a0
80050110:  lui v0,0x8006
80050114:  addu v0,v0,a2
80050118:  lbu v0,0x5120(v0)
8005011c:  move a0,a1
80050120:  sll v0,v0,0x2
80050124:  addu a1,a0,v0
80050128:  slt v0,v1,a1
8005012c:  bne v0,zero,0x80050148
80050130:  _slti v0,s0,0x13
80050134:  slti v0,v1,0xcbd
80050138:  beq v0,zero,0x80050144
8005013c:  _li a1,0xcbc
80050140:  move a1,v1
80050144:  slti v0,s0,0x13
80050148:  bne v0,zero,0x80050164
8005014c:  _move v1,a1
80050150:  slti v0,s0,0x130
80050154:  beq v0,zero,0x80050168
80050158:  _li a1,0x12f
8005015c:  j 0x80050168
80050160:  _move a1,s0
80050164:  li a1,0x13
