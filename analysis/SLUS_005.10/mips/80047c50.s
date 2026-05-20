# addr: 0x80047c50  name: _UDIVMOD_OBJ_41C
80047c50:  srlv v0,a0,v1
80047c54:  lui at,0x8001
80047c58:  addiu at,at,0xc64
80047c5c:  addu at,at,v0
80047c60:  lbu v0,0x0(at)
80047c64:  nop
80047c68:  addu v0,v0,v1
80047c6c:  ori v1,zero,0x20
80047c70:  subu t5,v1,v0
80047c74:  bne t5,zero,0x80047cc4
80047c78:  _subu t8,v1,t5
80047c7c:  sltu v0,a2,a1
80047c80:  bne v0,zero,0x80047c94
80047c84:  _ori t2,zero,0x1
80047c88:  sltu v0,t3,t1
80047c8c:  bne v0,zero,0x80047cac
80047c90:  _nop
80047c94:  subu a0,t3,t1
80047c98:  subu v1,a1,a2
80047c9c:  sltu v0,t3,a0
80047ca0:  subu a1,v1,v0
80047ca4:  j 0x80047cb0
80047ca8:  _move t3,a0
80047cac:  clear t2
