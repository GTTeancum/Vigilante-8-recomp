# addr: 0x80041fd4  name: FUN_80041fd4
80041fd4:  addiu sp,sp,-0x38
80041fd8:  sw s1,0x24(sp)
80041fdc:  lw s1,0x8c4(gp)
80041fe0:  sw ra,0x30(sp)
80041fe4:  sw s3,0x2c(sp)
80041fe8:  sw s2,0x28(sp)
80041fec:  sw s0,0x20(sp)
80041ff0:  lw s2,0x0(s1)
80041ff4:  nop
80041ff8:  beq s2,zero,0x80042088
80041ffc:  _lui v0,0x8007
80042000:  addiu s3,v0,-0x980
80042004:  lw a1,0x18(s1)
80042008:  addiu s0,s1,0xc
8004200c:  jal 0x8001db54
80042010:  _move a0,s0
80042014:  beq v0,zero,0x80042074
80042018:  _move a1,s0
8004201c:  move a0,s3
80042020:  jal 0x80043408
80042024:  _addiu a2,sp,0x10
80042028:  lw v1,0x18(sp)
8004202c:  lui v0,0x1f
80042030:  ori v0,v0,0xffff
80042034:  slt v0,v0,v1
80042038:  bne v0,zero,0x80042074
8004203c:  _nop
80042040:  jal 0x8004d344
80042044:  _move a0,s3
80042048:  lw a0,0x10(sp)
8004204c:  lw v1,0x14(sp)
80042050:  lw v0,0x18(sp)
80042054:  sra a0,a0,0x8
80042058:  sra v1,v1,0x8
8004205c:  sra v0,v0,0x8
80042060:  ldtr a0,v1,v0
8004206c:  jal 0x80040e38
80042070:  _move a0,s1
80042074:  move s1,s2
80042078:  lw s2,0x0(s2)
8004207c:  nop
80042080:  bne s2,zero,0x80042004
80042084:  _nop
80042088:  lw v0,0x8c0(gp)
8004208c:  nop
80042090:  blez v0,0x800420d8
80042094:  _clear s0
80042098:  lw v0,0x8d4(gp)
8004209c:  sll v1,s0,0x2
800420a0:  addu v1,v1,v0
800420a4:  lw a0,0x0(v1)
800420a8:  nop
800420ac:  lw v0,0x18(a0)
800420b0:  nop
800420b4:  beq v0,zero,0x800420c4
800420b8:  _nop
800420bc:  jal 0x80041c5c
800420c0:  _nop
800420c4:  lw v0,0x8c0(gp)
800420c8:  addiu s0,s0,0x1
800420cc:  slt v0,s0,v0
800420d0:  bne v0,zero,0x80042098
800420d4:  _nop
800420d8:  lw ra,0x30(sp)
800420dc:  lw s3,0x2c(sp)
800420e0:  lw s2,0x28(sp)
800420e4:  lw s1,0x24(sp)
800420e8:  lw s0,0x20(sp)
800420ec:  jr ra
800420f0:  _addiu sp,sp,0x38
