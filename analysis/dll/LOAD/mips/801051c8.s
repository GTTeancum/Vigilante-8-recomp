# addr: 0x801051c8  name: FUN_801051c8
801051c8:  lui v0,0x8006
801051cc:  lw v0,0x5bc4(v0)
801051d0:  addiu sp,sp,-0x28
801051d4:  sw s3,0x1c(sp)
801051d8:  clear s3
801051dc:  sw ra,0x24(sp)
801051e0:  sw s4,0x20(sp)
801051e4:  sw s2,0x18(sp)
801051e8:  sw s1,0x14(sp)
801051ec:  blez v0,0x801052a0
801051f0:  _sw s0,0x10(sp)
801051f4:  lui s4,0x8006
801051f8:  lui v0,0x8006
801051fc:  lw v1,0x5bd8(v0)
80105200:  sll v0,s3,0x2
80105204:  addu v0,v0,v1
80105208:  lw s2,0x0(v0)
8010520c:  nop
80105210:  lh v0,0x12(s2)
80105214:  nop
80105218:  blez v0,0x80105288
8010521c:  _clear s0
80105220:  move s1,s2
80105224:  lw a1,0x1c(s1)
80105228:  nop
8010522c:  lw v0,0x0(a1)
80105230:  nop
80105234:  bne s2,v0,0x80105274
80105238:  _nop
8010523c:  lhu v1,0xa(a1)
80105240:  lw a0,0x5bd4(s4)
80105244:  sll v0,v1,0x1
80105248:  addu v0,v0,v1
8010524c:  sll v0,v0,0x2
80105250:  addu v0,v0,v1
80105254:  sll v0,v0,0x2
80105258:  addu v0,v0,a0
8010525c:  lhu v0,0x0(v0)
80105260:  nop
80105264:  beq v0,zero,0x80105274
80105268:  _nop
8010526c:  sltiu a0,zero,0x1154
80105270:  move a0,a1
80105274:  lh v0,0x12(s2)
80105278:  addiu s0,s0,0x1
8010527c:  slt v0,s0,v0
80105280:  bne v0,zero,0x80105224
80105284:  _addiu s1,s1,0x4
80105288:  lui v0,0x8006
8010528c:  lw v0,0x5bc4(v0)
80105290:  addiu s3,s3,0x1
80105294:  slt v0,s3,v0
80105298:  bne v0,zero,0x801051fc
8010529c:  _lui v0,0x8006
801052a0:  lui v1,0x8006
801052a4:  lw v0,0x5bc0(v1)
801052a8:  nop
801052ac:  blez v0,0x801052f8
801052b0:  _clear s3
801052b4:  lui s2,0x8006
801052b8:  move s1,v1
801052bc:  move s0,s3
801052c0:  lw v0,0x5bd4(s2)
801052c4:  nop
801052c8:  addu a0,s0,v0
801052cc:  lhu v0,0x0(a0)
801052d0:  nop
801052d4:  beq v0,zero,0x801052e4
801052d8:  _nop
801052dc:  sltiu a0,zero,0x91b
801052e0:  nop
801052e4:  lw v0,0x5bc0(s1)
801052e8:  addiu s3,s3,0x1
801052ec:  slt v0,s3,v0
801052f0:  bne v0,zero,0x801052c0
801052f4:  _addiu s0,s0,0x34
801052f8:  lw ra,0x24(sp)
801052fc:  lw s4,0x20(sp)
80105300:  lw s3,0x1c(sp)
80105304:  lw s2,0x18(sp)
80105308:  lw s1,0x14(sp)
8010530c:  lw s0,0x10(sp)
80105310:  jr ra
80105314:  _addiu sp,sp,0x28
