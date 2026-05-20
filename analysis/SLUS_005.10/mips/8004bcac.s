# addr: 0x8004bcac  name: C_011_OBJ_258
8004bcac:  lui a0,0x8006
8004bcb0:  lw a0,0x464(a0)
8004bcb4:  nop
8004bcb8:  lw v0,0x0(a0)
8004bcbc:  lui v1,0x100
8004bcc0:  and v0,v0,v1
8004bcc4:  beq v0,zero,0x8004bce4
8004bcc8:  _move v1,a0
8004bccc:  lui a0,0x100
8004bcd0:  lw v0,0x0(v1)
8004bcd4:  nop
8004bcd8:  and v0,v0,a0
8004bcdc:  bne v0,zero,0x8004bcd0
8004bce0:  _nop
8004bce4:  lui a0,0x2
8004bce8:  ori a0,a0,0x843
8004bcec:  lui v0,0x800a
8004bcf0:  lw v0,0x32d8(v0)
8004bcf4:  lui v1,0x8006
8004bcf8:  lw v1,0x444(v1)
8004bcfc:  lwl a1,0x2b(sp)
8004bd00:  lwr a1,0x28(sp)
8004bd04:  nop
8004bd08:  swl a1,0x1f(v0)
8004bd0c:  swr a1,0x1c(v0)
8004bd10:  sw a0,0x0(v1)
8004bd14:  lui v1,0x8006
8004bd18:  lw v1,0x448(v1)
8004bd1c:  li v0,0x1325
8004bd20:  sw v0,0x0(v1)
8004bd24:  lui v1,0x800a
8004bd28:  lw v1,0x32c0(v1)
8004bd2c:  li v0,0x1
8004bd30:  bne v1,v0,0x8004bda4
8004bd34:  _nop
8004bd38:  lui a0,0x800a
8004bd3c:  lw a0,0x329c(a0)
8004bd40:  nop
8004bd44:  beq a0,zero,0x8004bda4
8004bd48:  _nop
8004bd4c:  lui v1,0x800a
8004bd50:  lw v1,0x32d8(v1)
8004bd54:  nop
8004bd58:  lhu v0,0x8(v1)
8004bd5c:  nop
8004bd60:  beq a0,v0,0x8004bd9c
8004bd64:  _nop
8004bd68:  sh zero,0x0(v1)
8004bd6c:  lui v0,0x800a
8004bd70:  lw v0,0x32b8(v0)
8004bd74:  nop
8004bd78:  beq v0,zero,0x8004c360
8004bd7c:  _nop
8004bd80:  lui v0,0x800a
8004bd84:  lw v0,0x32a8(v0)
8004bd88:  nop
8004bd8c:  addiu v0,v0,0x1
8004bd90:  lui at,0x800a
8004bd94:  j 0x8004c360
8004bd98:  _sw v0,0x32a8(at)
8004bd9c:  lui at,0x800a
8004bda0:  sw zero,0x32c0(at)
8004bda4:  lui a0,0x800a
8004bda8:  lw a0,0x32d8(a0)
8004bdac:  nop
8004bdb0:  lhu v1,0x0(a0)
8004bdb4:  li v0,0x160
8004bdb8:  bne v1,v0,0x8004bddc
8004bdbc:  _nop
8004bdc0:  lhu v0,0x2(a0)
8004bdc4:  lui v1,0x800a
8004bdc8:  lw v1,0x32a0(v1)
8004bdcc:  srl v0,v0,0xa
8004bdd0:  andi v0,v0,0x1f
8004bdd4:  beq v0,v1,0x8004be1c
8004bdd8:  _nop
8004bddc:  lui v0,0x800a
8004bde0:  lw v0,0x32b8(v0)
8004bde4:  nop
8004bde8:  beq v0,zero,0x8004bdfc
8004bdec:  _nop
8004bdf0:  lui at,0x800a
8004bdf4:  j 0x8004be00
8004bdf8:  _sw zero,0x32a8(at)
8004bdfc:  lhu v0,0x0(a0)
