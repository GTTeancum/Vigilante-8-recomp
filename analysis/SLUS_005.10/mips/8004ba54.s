# addr: 0x8004ba54  name: StCdInterrupt
8004ba54:  addiu sp,sp,-0x40
8004ba58:  lui v0,0x800a
8004ba5c:  lw v0,0x32a4(v0)
8004ba60:  li a0,0x1
8004ba64:  beq v0,a0,0x8004c360
8004ba68:  _sw ra,0x38(sp)
8004ba6c:  lui v0,0x800a
8004ba70:  lw v0,0x3290(v0)
8004ba74:  nop
8004ba78:  beq v0,zero,0x8004bad8
8004ba7c:  _nop
8004ba80:  lui v0,0x8006
8004ba84:  lw v0,0x454(v0)
8004ba88:  nop
8004ba8c:  lw v0,0x0(v0)
8004ba90:  lui v1,0x100
8004ba94:  and v0,v0,v1
8004ba98:  beq v0,zero,0x8004bad8
8004ba9c:  _nop
8004baa0:  lui v0,0x800a
8004baa4:  lw v0,0x32b8(v0)
8004baa8:  lui at,0x800a
8004baac:  beq v0,zero,0x8004bacc
8004bab0:  _sw a0,0x3294(at)
8004bab4:  lui v0,0x800a
8004bab8:  lw v0,0x32a8(v0)
8004babc:  nop
8004bac0:  addiu v0,v0,0x1
8004bac4:  lui at,0x800a
8004bac8:  sw v0,0x32a8(at)
8004bacc:  lui at,0x8006
8004bad0:  j 0x8004c360
8004bad4:  _sw a0,0x47c(at)
8004bad8:  jal 0x80048f88
8004badc:  _addiu a1,sp,0x30
8004bae0:  li v1,0x5
8004bae4:  beq v0,v1,0x8004c360
8004bae8:  _nop
8004baec:  lbu v0,0x30(sp)
8004baf0:  lbu v1,0x31(sp)
8004baf4:  sh v0,0x22(sp)
8004baf8:  sh v1,0x24(sp)
8004bafc:  lhu v0,0x22(sp)
8004bb00:  nop
8004bb04:  andi v0,v0,0x4
8004bb08:  beq v0,zero,0x8004bb1c
8004bb0c:  _li v0,0x3
8004bb10:  lui at,0x8006
8004bb14:  j 0x8004c360
8004bb18:  _sw v0,0x47c(at)
8004bb1c:  lui v0,0x800a
8004bb20:  lw v0,0x32ac(v0)
8004bb24:  lui v1,0x800a
8004bb28:  lw v1,0x32c8(v1)
8004bb2c:  sll v0,v0,0x5
8004bb30:  addu v1,v1,v0
8004bb34:  lui at,0x800a
8004bb38:  sw v1,0x32d8(at)
8004bb3c:  lhu v0,0x0(v1)
8004bb40:  nop
8004bb44:  beq v0,zero,0x8004bb88
8004bb48:  _nop
8004bb4c:  lui v0,0x800a
8004bb50:  lw v0,0x32b8(v0)
8004bb54:  nop
8004bb58:  beq v0,zero,0x8004bb7c
8004bb5c:  _li v0,0x4
8004bb60:  lui v0,0x800a
8004bb64:  lw v0,0x32a8(v0)
8004bb68:  nop
8004bb6c:  addiu v0,v0,0x1
8004bb70:  lui at,0x800a
8004bb74:  sw v0,0x32a8(at)
8004bb78:  li v0,0x4
8004bb7c:  lui at,0x8006
8004bb80:  j 0x8004c360
8004bb84:  _sw v0,0x47c(at)
8004bb88:  lui v0,0x8006
8004bb8c:  lw v0,0x434(v0)
8004bb90:  nop
8004bb94:  sb zero,0x0(v0)
8004bb98:  lui v0,0x8006
8004bb9c:  lw v0,0x440(v0)
8004bba0:  nop
8004bba4:  sb zero,0x0(v0)
8004bba8:  lui v0,0x8006
8004bbac:  lw v0,0x434(v0)
8004bbb0:  lui a0,0x2
8004bbb4:  sb zero,0x0(v0)
8004bbb8:  lui v1,0x8006
8004bbbc:  lw v1,0x440(v1)
8004bbc0:  li v0,0x80
8004bbc4:  sb v0,0x0(v1)
8004bbc8:  lui v0,0x8006
8004bbcc:  lw v0,0x444(v0)
8004bbd0:  ori a0,a0,0x943
8004bbd4:  sw a0,0x0(v0)
8004bbd8:  lui v1,0x8006
8004bbdc:  lw v1,0x448(v1)
8004bbe0:  li v0,0x1323
8004bbe4:  sw v0,0x0(v1)
8004bbe8:  lui v0,0x800a
8004bbec:  lw v0,0x3270(v0)
8004bbf0:  nop
8004bbf4:  bne v0,zero,0x8004bc48
8004bbf8:  _clear a0
8004bbfc:  addiu a1,sp,0x28
8004bc00:  lui v0,0x8006
8004bc04:  lw v0,0x43c(v0)
8004bc08:  addu v1,a1,a0
8004bc0c:  lbu v0,0x0(v0)
8004bc10:  addiu a0,a0,0x1
8004bc14:  sb v0,0x0(v1)
8004bc18:  sltiu v0,a0,0x4
8004bc1c:  bne v0,zero,0x8004bc00
8004bc20:  _nop
8004bc24:  clear a0
8004bc28:  lui v1,0x8006
8004bc2c:  lw v1,0x43c(v1)
8004bc30:  nop
8004bc34:  lbu v0,0x0(v1)
8004bc38:  addiu a0,a0,0x1
8004bc3c:  sltiu v0,a0,0x8
8004bc40:  bne v0,zero,0x8004bc34
8004bc44:  _nop
8004bc48:  lui v0,0x800a
8004bc4c:  lw v0,0x32b8(v0)
8004bc50:  nop
8004bc54:  beq v0,zero,0x8004bc88
8004bc58:  _lui t0,0x1100
8004bc5c:  li a2,0x8
8004bc60:  clear a3
8004bc64:  lui a1,0x800a
8004bc68:  lw a1,0x32a8(a1)
8004bc6c:  lui a0,0x800a
8004bc70:  lw a0,0x32d8(a0)
8004bc74:  sll a1,a1,0xb
8004bc78:  jal 0x8004c370
8004bc7c:  _addu a1,v0,a1
8004bc80:  j 0x8004bcac
8004bc84:  _nop
8004bc88:  li a0,0x3
8004bc8c:  clear a2
8004bc90:  lui a1,0x800a
8004bc94:  lw a1,0x32d8(a1)
8004bc98:  li a3,0x8
8004bc9c:  sw t0,0x10(sp)
8004bca0:  sw zero,0x14(sp)
8004bca4:  jal 0x8004c39c
8004bca8:  _sw zero,0x18(sp)
