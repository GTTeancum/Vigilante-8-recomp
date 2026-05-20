# addr: 0x8004aa44  name: CD_getsector
8004aa44:  lui v0,0x8006
8004aa48:  lw v0,0x344(v0)
8004aa4c:  lui a2,0x2
8004aa50:  sb zero,0x0(v0)
8004aa54:  lui v1,0x8006
8004aa58:  lw v1,0x350(v1)
8004aa5c:  li v0,0x80
8004aa60:  sb v0,0x0(v1)
8004aa64:  lui v0,0x8006
8004aa68:  lw v0,0x378(v0)
8004aa6c:  ori a2,a2,0x943
8004aa70:  sw a2,0x0(v0)
8004aa74:  lui v1,0x8006
8004aa78:  lw v1,0x354(v1)
8004aa7c:  li v0,0x1323
8004aa80:  sw v0,0x0(v1)
8004aa84:  lui v1,0x8006
8004aa88:  lw v1,0x37c(v1)
8004aa8c:  nop
8004aa90:  lw v0,0x0(v1)
8004aa94:  nop
8004aa98:  ori v0,v0,0x8000
8004aa9c:  sw v0,0x0(v1)
8004aaa0:  lui v0,0x8006
8004aaa4:  lw v0,0x380(v0)
8004aaa8:  nop
8004aaac:  sw a0,0x0(v0)
8004aab0:  lui v0,0x1
8004aab4:  lui v1,0x8006
8004aab8:  lw v1,0x384(v1)
8004aabc:  or a1,a1,v0
8004aac0:  sw a1,0x0(v1)
8004aac4:  lui v1,0x8006
8004aac8:  lw v1,0x344(v1)
8004aacc:  nop
8004aad0:  lbu v0,0x0(v1)
8004aad4:  nop
8004aad8:  andi v0,v0,0x40
8004aadc:  beq v0,zero,0x8004aad0
8004aae0:  _lui v0,0x1100
8004aae4:  lui v1,0x8006
8004aae8:  lw v1,0x388(v1)
8004aaec:  nop
8004aaf0:  sw v0,0x0(v1)
8004aaf4:  lui a0,0x8006
8004aaf8:  lw a0,0x388(a0)
8004aafc:  nop
8004ab00:  lw v0,0x0(a0)
8004ab04:  lui v1,0x100
8004ab08:  and v0,v0,v1
8004ab0c:  beq v0,zero,0x8004ab2c
8004ab10:  _move v1,a0
8004ab14:  lui a0,0x100
8004ab18:  lw v0,0x0(v1)
8004ab1c:  nop
8004ab20:  and v0,v0,a0
8004ab24:  bne v0,zero,0x8004ab18
8004ab28:  _nop
8004ab2c:  lui v1,0x8006
8004ab30:  lw v1,0x354(v1)
8004ab34:  li v0,0x1325
8004ab38:  sw v0,0x0(v1)
8004ab3c:  jr ra
8004ab40:  _clear v0
