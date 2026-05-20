# addr: 0x80046bb0  name: S_SCA_OBJ_31C
80046bb0:  sh v0,0x1aa(v1)
80046bb4:  bne t2,zero,0x80046bc4
80046bb8:  _andi v0,t1,0x2000
80046bbc:  beq v0,zero,0x80046c08
80046bc0:  _nop
80046bc4:  lw v0,0x24(a0)
80046bc8:  nop
80046bcc:  bne v0,zero,0x80046bec
80046bd0:  _nop
80046bd4:  lui v1,0x8006
80046bd8:  lw v1,-0x122c(v1)
80046bdc:  nop
80046be0:  lhu v0,0x1aa(v1)
80046be4:  j 0x80046c04
80046be8:  _andi v0,v0,0xfffd
80046bec:  lui v1,0x8006
80046bf0:  lw v1,-0x122c(v1)
80046bf4:  nop
80046bf8:  lhu v0,0x1aa(v1)
80046bfc:  nop
80046c00:  ori v0,v0,0x2
