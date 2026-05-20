# addr: 0x800509c8  name: SYS_OBJ_17C4
800509c8:  sll v0,a0,0x10
800509cc:  sra a2,v0,0x10
800509d0:  bltz a2,0x80050a00
800509d4:  _clear v0
800509d8:  lui v0,0x8006
800509dc:  lh v0,0x5028(v0)
800509e0:  nop
800509e4:  addiu v0,v0,-0x1
800509e8:  slt v0,v0,a2
800509ec:  lui a2,0x8006
800509f0:  lhu a2,0x5028(a2)
800509f4:  bne v0,zero,0x80050a00
800509f8:  _addiu v0,a2,-0x1
800509fc:  move v0,a0
80050a00:  move a0,v0
80050a04:  sll v0,a1,0x10
80050a08:  sra a2,v0,0x10
80050a0c:  bltz a2,0x80050a40
80050a10:  _nop
80050a14:  lui v0,0x8006
80050a18:  lh v0,0x502a(v0)
80050a1c:  nop
80050a20:  addiu v0,v0,-0x1
80050a24:  slt v0,v0,a2
80050a28:  lui a2,0x8006
80050a2c:  lhu a2,0x502a(a2)
80050a30:  beq v0,zero,0x80050a48
80050a34:  _andi v1,a1,0x3ff
80050a38:  j 0x80050a44
80050a3c:  _addiu a1,a2,-0x1
80050a40:  clear a1
