# addr: 0x80015bf0  name: FUN_80015bf0
80015bf0:  addiu sp,sp,-0x20
80015bf4:  sw s1,0x14(sp)
80015bf8:  move s1,a0
80015bfc:  sw ra,0x1c(sp)
80015c00:  sw s2,0x18(sp)
80015c04:  beq a1,zero,0x80015c18
80015c08:  _sw s0,0x10(sp)
80015c0c:  lw v0,0x6ac(gp)
80015c10:  nop
80015c14:  addu s1,s1,v0
80015c18:  lw v0,0x6ac(gp)
80015c1c:  srl v1,s1,0xb
80015c20:  sra v0,v0,0xb
80015c24:  subu s0,v1,v0
80015c28:  addiu s0,s0,-0x1
80015c2c:  li v0,-0x1
80015c30:  beq s0,v0,0x80015c4c
80015c34:  _move s2,v0
80015c38:  jal 0x800156d4
80015c3c:  _addiu s0,s0,-0x1
80015c40:  sw v0,0x69c(gp)
80015c44:  bne s0,s2,0x80015c38
80015c48:  _nop
80015c4c:  lw ra,0x1c(sp)
80015c50:  lw s2,0x18(sp)
80015c54:  sw s1,0x6ac(gp)
80015c58:  lw s1,0x14(sp)
80015c5c:  lw s0,0x10(sp)
80015c60:  jr ra
80015c64:  _addiu sp,sp,0x20
