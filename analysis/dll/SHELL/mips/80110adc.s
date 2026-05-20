# addr: 0x80110adc  name: FUN_80110adc
80110adc:  addiu sp,sp,-0x30
80110ae0:  sw s2,0x18(sp)
80110ae4:  move s2,a0
80110ae8:  sw s4,0x20(sp)
80110aec:  move s4,a1
80110af0:  sw ra,0x28(sp)
80110af4:  sw s5,0x24(sp)
80110af8:  sw s3,0x1c(sp)
80110afc:  sw s1,0x14(sp)
80110b00:  sw s0,0x10(sp)
80110b04:  lw v0,0x0(s2)
80110b08:  nop
80110b0c:  bne v0,zero,0x80110b1c
80110b10:  _move s5,a2
80110b14:  li v0,0x1
80110b18:  sw v0,0x0(s2)
80110b1c:  lw v1,0x154(zero)
80110b20:  lui v0,0xcccc
80110b24:  ori v0,v0,0xcccd
80110b28:  multu v1,v0
80110b2c:  lw s0,0x150(zero)
80110b30:  lui s3,0x8011
80110b34:  lw s3,0x3490(s3)
80110b38:  mfhi a3
80110b3c:  srl v1,a3,0x6
80110b40:  sll v0,v1,0x2
80110b44:  addu v0,v0,v1
80110b48:  sll v0,v0,0x4
80110b4c:  addu v1,s0,v0
80110b50:  sltu v0,s0,v1
80110b54:  beq v0,zero,0x80110ba0
80110b58:  _move a0,s2
80110b5c:  move s1,v1
80110b60:  lw a0,0x0(s0)
80110b64:  nop
80110b68:  beq a0,zero,0x80110b8c
80110b6c:  _nop
80110b70:  lui a1,0x8011
80110b74:  jal 0x80052484
80110b78:  _addiu a1,a1,0x3498
80110b7c:  bne v0,zero,0x80110b8c
80110b80:  _nop
80110b84:  slti a0,zero,0x42e7
80110b88:  sw s3,0x34(s0)
80110b8c:  addiu s0,s0,0x50
80110b90:  sltu v0,s0,s1
80110b94:  bne v0,zero,0x80110b60
80110b98:  _nop
80110b9c:  move a0,s2
80110ba0:  move a1,s4
80110ba4:  lui v0,0x8011
80110ba8:  lw v0,0x3490(v0)
80110bac:  nop
80110bb0:  jalr v0
80110bb4:  _move a2,s5
80110bb8:  lw ra,0x28(sp)
80110bbc:  lw s5,0x24(sp)
80110bc0:  lw s4,0x20(sp)
80110bc4:  lw s3,0x1c(sp)
80110bc8:  lw s2,0x18(sp)
80110bcc:  lw s1,0x14(sp)
80110bd0:  lw s0,0x10(sp)
80110bd4:  jr ra
80110bd8:  _addiu sp,sp,0x30
