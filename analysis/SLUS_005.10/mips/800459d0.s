# addr: 0x800459d0  name: _spu_Fr_
800459d0:  lui v0,0x8006
800459d4:  lw v0,-0x122c(v0)
800459d8:  addiu sp,sp,-0x20
800459dc:  sw s1,0x14(sp)
800459e0:  move s1,a0
800459e4:  sw s0,0x10(sp)
800459e8:  sw ra,0x18(sp)
800459ec:  sh a1,0x1a6(v0)
800459f0:  jal 0x80045fac
800459f4:  _move s0,a2
800459f8:  lui v1,0x8006
800459fc:  lw v1,-0x122c(v1)
80045a00:  nop
80045a04:  lhu v0,0x1aa(v1)
80045a08:  nop
80045a0c:  ori v0,v0,0x30
80045a10:  sh v0,0x1aa(v1)
80045a14:  jal 0x80045fac
80045a18:  _sll s0,s0,0x10
80045a1c:  jal 0x80045f84
80045a20:  _nop
80045a24:  lui a0,0x100
80045a28:  ori a0,a0,0x200
80045a2c:  lui v0,0x8006
80045a30:  lw v0,-0x1228(v0)
80045a34:  nop
80045a38:  sw s1,0x0(v0)
80045a3c:  lui v0,0x8006
80045a40:  lw v0,-0x1224(v0)
80045a44:  ori s0,s0,0x10
80045a48:  sw s0,0x0(v0)
80045a4c:  lui v1,0x8006
80045a50:  lw v1,-0x1220(v1)
80045a54:  li v0,0x1
80045a58:  lui at,0x8006
80045a5c:  sw v0,-0x11dc(at)
80045a60:  sw a0,0x0(v1)
80045a64:  lw ra,0x18(sp)
80045a68:  lw s1,0x14(sp)
80045a6c:  lw s0,0x10(sp)
80045a70:  jr ra
80045a74:  _addiu sp,sp,0x20
