# addr: 0x80055f30  name: PADPORTD_OBJ_35C
80055f30:  lbu v0,0x35(a0)
80055f34:  nop
80055f38:  slt v0,v1,v0
80055f3c:  beq v0,zero,0x80055f54
80055f40:  _clear v0
80055f44:  lw v0,0x2c(a0)
80055f48:  nop
80055f4c:  addu v0,v0,v1
80055f50:  lbu v0,0x0(v0)
