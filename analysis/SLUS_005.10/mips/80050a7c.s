# addr: 0x80050a7c  name: SYS_OBJ_1878
80050a7c:  bne a0,zero,0x80050a8c
80050a80:  _addiu sp,sp,-0x10
80050a84:  j 0x80050af4
80050a88:  _clear v0
80050a8c:  lbu a1,0x0(a0)
80050a90:  nop
80050a94:  srl a1,a1,0x3
80050a98:  sw a1,0x0(sp)
80050a9c:  lh a2,0x4(a0)
80050aa0:  nop
80050aa4:  subu a2,zero,a2
80050aa8:  andi a2,a2,0xff
80050aac:  sra a2,a2,0x3
80050ab0:  sw a2,0x8(sp)
80050ab4:  lbu v0,0x2(a0)
80050ab8:  sll a1,a1,0xa
80050abc:  srl v0,v0,0x3
80050ac0:  sw v0,0x4(sp)
80050ac4:  sll v0,v0,0xf
80050ac8:  lh v1,0x6(a0)
80050acc:  lui a0,0xe200
80050ad0:  or a1,a1,a0
80050ad4:  or v0,v0,a1
80050ad8:  subu v1,zero,v1
80050adc:  andi v1,v1,0xff
80050ae0:  sra v1,v1,0x3
80050ae4:  sll a0,v1,0x5
80050ae8:  or v0,v0,a0
80050aec:  or v0,v0,a2
80050af0:  sw v1,0xc(sp)
