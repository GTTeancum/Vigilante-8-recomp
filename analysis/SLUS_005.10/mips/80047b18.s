# addr: 0x80047b18  name: _UDIVMOD_OBJ_2E4
80047b18:  srl a2,t1,0x10
80047b1c:  divu a1,a2
80047b20:  bne a2,zero,0x80047b2c
80047b24:  _nop
80047b28:  break 0x1c00
80047b2c:  mflo a3
80047b30:  mfhi v1
80047b34:  andi t0,t1,0xffff
80047b38:  mult a3,t0
80047b3c:  sll v0,v1,0x10
80047b40:  srl v1,t3,0x10
80047b44:  or v1,v0,v1
80047b48:  mflo a0
80047b4c:  sltu v0,v1,a0
80047b50:  beq v0,zero,0x80047b7c
80047b54:  _nop
80047b58:  addu v1,v1,t1
80047b5c:  sltu v0,v1,t1
80047b60:  bne v0,zero,0x80047b7c
80047b64:  _addiu a3,a3,-0x1
80047b68:  sltu v0,v1,a0
80047b6c:  beq v0,zero,0x80047b7c
80047b70:  _nop
80047b74:  addiu a3,a3,-0x1
80047b78:  addu v1,v1,t1
80047b7c:  subu v1,v1,a0
80047b80:  divu v1,a2
80047b84:  bne a2,zero,0x80047b90
80047b88:  _nop
80047b8c:  break 0x1c00
80047b90:  mflo a1
80047b94:  mfhi v1
80047b98:  mult a1,t0
80047b9c:  sll v0,v1,0x10
80047ba0:  andi v1,t3,0xffff
80047ba4:  or v1,v0,v1
80047ba8:  mflo a0
80047bac:  sltu v0,v1,a0
80047bb0:  beq v0,zero,0x80047be0
80047bb4:  _sll v0,a3,0x10
80047bb8:  addu v1,v1,t1
80047bbc:  sltu v0,v1,t1
80047bc0:  bne v0,zero,0x80047bdc
80047bc4:  _addiu a1,a1,-0x1
80047bc8:  sltu v0,v1,a0
80047bcc:  beq v0,zero,0x80047be0
80047bd0:  _sll v0,a3,0x10
80047bd4:  addiu a1,a1,-0x1
80047bd8:  addu v1,v1,t1
80047bdc:  sll v0,a3,0x10
80047be0:  or t2,v0,a1
80047be4:  subu t3,v1,a0
