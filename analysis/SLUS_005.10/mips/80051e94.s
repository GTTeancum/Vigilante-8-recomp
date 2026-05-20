# addr: 0x80051e94  name: StoreImage2
80051e94:  addiu sp,sp,-0x20
80051e98:  sw s0,0x10(sp)
80051e9c:  move s0,a0
80051ea0:  sw s1,0x14(sp)
80051ea4:  move s1,a1
80051ea8:  lui a0,0x8001
80051eac:  addiu a0,a0,0x1298
80051eb0:  sw ra,0x18(sp)
80051eb4:  jal 0x8004f5e8
80051eb8:  _move a1,s0
80051ebc:  jal 0x80047e44
80051ec0:  _li a0,-0x1
80051ec4:  lui v1,0x8006
80051ec8:  lw v1,0x5138(v1)
80051ecc:  addiu v0,v0,0xf0
80051ed0:  lui at,0x8006
80051ed4:  sw v0,0x5160(at)
80051ed8:  lui at,0x8006
80051edc:  sw zero,0x5164(at)
80051ee0:  lw v0,0x0(v1)
80051ee4:  j 0x80051f10
80051ee8:  _lui v1,0x100
80051eec:  jal 0x80051bc4
80051ef0:  _nop
80051ef4:  bne v0,zero,0x80051f6c
80051ef8:  _li v0,-0x1
80051efc:  lui v0,0x8006
80051f00:  lw v0,0x5138(v0)
80051f04:  nop
80051f08:  lw v0,0x0(v0)
80051f0c:  lui v1,0x100
