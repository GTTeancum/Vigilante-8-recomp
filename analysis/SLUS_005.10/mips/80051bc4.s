# addr: 0x80051bc4  name: SYS_OBJ_29C0
80051bc4:  addiu sp,sp,-0x20
80051bc8:  sw ra,0x18(sp)
80051bcc:  jal 0x80047e44
80051bd0:  _li a0,-0x1
80051bd4:  lui v1,0x8006
80051bd8:  lw v1,0x5160(v1)
80051bdc:  nop
80051be0:  slt v1,v1,v0
80051be4:  bne v1,zero,0x80051c18
80051be8:  _nop
80051bec:  lui v1,0x8006
80051bf0:  addiu v1,v1,0x5164
80051bf4:  lw v0,0x0(v1)
80051bf8:  nop
80051bfc:  move a0,v0
80051c00:  addiu v0,v0,0x1
80051c04:  sw v0,0x0(v1)
80051c08:  lui v0,0xf
80051c0c:  slt v0,v0,a0
80051c10:  beq v0,zero,0x80051cf4
80051c14:  _nop
80051c18:  lui a2,0x8006
80051c1c:  lw a2,0x512c(a2)
80051c20:  lui a0,0x8001
80051c24:  addiu a0,a0,0x1340
80051c28:  lw v0,0x0(a2)
80051c2c:  lui a1,0x8006
80051c30:  lw a1,0x514c(a1)
80051c34:  lui v0,0x8006
80051c38:  lw v0,0x5130(v0)
80051c3c:  lui v1,0x8006
80051c40:  lw v1,0x5150(v1)
80051c44:  lw v0,0x0(v0)
80051c48:  subu a1,a1,v1
80051c4c:  sw v0,0x10(sp)
80051c50:  lui v0,0x8006
80051c54:  lw v0,0x5138(v0)
80051c58:  lw a2,0x0(a2)
80051c5c:  lw a3,0x0(v0)
80051c60:  jal 0x80052604
80051c64:  _andi a1,a1,0x3f
80051c68:  jal 0x800481d0
80051c6c:  _clear a0
80051c70:  lui at,0x8006
80051c74:  sw zero,0x5150(at)
80051c78:  lui v1,0x8006
80051c7c:  lw v1,0x5150(v1)
80051c80:  lui at,0x8006
80051c84:  sw v0,0x515c(at)
80051c88:  lui at,0x8006
80051c8c:  sw v1,0x514c(at)
80051c90:  lui v1,0x8006
80051c94:  lw v1,0x5138(v1)
80051c98:  li v0,0x401
80051c9c:  sw v0,0x0(v1)
80051ca0:  lui v1,0x8006
80051ca4:  lw v1,0x5148(v1)
80051ca8:  nop
80051cac:  lw v0,0x0(v1)
80051cb0:  nop
80051cb4:  ori v0,v0,0x800
80051cb8:  sw v0,0x0(v1)
80051cbc:  lui v1,0x8006
80051cc0:  lw v1,0x512c(v1)
80051cc4:  lui v0,0x200
80051cc8:  sw v0,0x0(v1)
80051ccc:  lui v1,0x8006
80051cd0:  lw v1,0x512c(v1)
80051cd4:  lui v0,0x100
80051cd8:  sw v0,0x0(v1)
80051cdc:  lui a0,0x8006
80051ce0:  lw a0,0x515c(a0)
80051ce4:  jal 0x800481d0
80051ce8:  _nop
80051cec:  j 0x80051cf8
80051cf0:  _li v0,-0x1
80051cf4:  clear v0
