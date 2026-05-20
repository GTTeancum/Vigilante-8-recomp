# addr: 0x80051a8c  name: SYS_OBJ_2888
80051a8c:  lui v1,0x8006
80051a90:  lw v1,0x514c(v1)
80051a94:  lui v0,0x8006
80051a98:  lw v0,0x5150(v0)
80051a9c:  nop
80051aa0:  beq v1,v0,0x80051ac0
80051aa4:  _nop
80051aa8:  j 0x80051a74
80051aac:  _nop
80051ab0:  jal 0x80051bc4
80051ab4:  _nop
80051ab8:  bne v0,zero,0x80051b80
80051abc:  _li v0,-0x1
80051ac0:  lui v0,0x8006
80051ac4:  lw v0,0x5138(v0)
80051ac8:  nop
80051acc:  lw v0,0x0(v0)
80051ad0:  lui v1,0x100
80051ad4:  and v0,v0,v1
80051ad8:  bne v0,zero,0x80051ab0
80051adc:  _nop
80051ae0:  lui v0,0x8006
80051ae4:  lw v0,0x512c(v0)
80051ae8:  nop
80051aec:  lw v0,0x0(v0)
80051af0:  lui v1,0x400
80051af4:  and v0,v0,v1
80051af8:  beq v0,zero,0x80051ab0
80051afc:  _clear v0
80051b00:  j 0x80051b80
80051b04:  _nop
