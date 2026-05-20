# addr: 0x8002ad30  name: FUN_8002ad30
8002ad30:  lw v1,0x714(gp)
8002ad34:  addiu sp,sp,-0x38
8002ad38:  sw ra,0x34(sp)
8002ad3c:  sw s8,0x30(sp)
8002ad40:  sw s7,0x2c(sp)
8002ad44:  sw s6,0x28(sp)
8002ad48:  sw s5,0x24(sp)
8002ad4c:  sw s4,0x20(sp)
8002ad50:  sw s3,0x1c(sp)
8002ad54:  sw s2,0x18(sp)
8002ad58:  sw s1,0x14(sp)
8002ad5c:  sw s0,0x10(sp)
8002ad60:  sw a3,0x44(sp)
8002ad64:  lw s5,0x0(v1)
8002ad68:  move s4,a0
8002ad6c:  move s7,a1
8002ad70:  beq s5,zero,0x8002af68
8002ad74:  _move s8,a2
8002ad78:  lui v0,0x800a
8002ad7c:  addiu s6,v0,0x2bb8
8002ad80:  lw s0,0x8(v1)
8002ad84:  nop
8002ad88:  beq s0,s4,0x8002af58
8002ad8c:  _move v1,s5
8002ad90:  lbu v1,0x4(s0)
8002ad94:  li v0,0x3
8002ad98:  beq v1,v0,0x8002af58
8002ad9c:  _move v1,s5
8002ada0:  lw v0,0x0(s0)
8002ada4:  nop
8002ada8:  andi v0,v0,0x4000
8002adac:  beq v0,zero,0x8002af58
8002adb0:  _nop
8002adb4:  lw a0,0x48(s0)
8002adb8:  lw v1,0x24(s4)
8002adbc:  lw v0,0x3bc(gp)
8002adc0:  subu a0,a0,v1
8002adc4:  addiu v0,v0,0x1
8002adc8:  andi v0,v0,0x3f
8002adcc:  sw v0,0x3bc(gp)
8002add0:  sll v0,v0,0x4
8002add4:  bgez a0,0x8002ade8
8002add8:  _addu s3,v0,s6
8002addc:  lui v0,0x1
8002ade0:  ori v0,v0,0xffff
8002ade4:  addu a0,a0,v0
8002ade8:  lw v1,0x50(s0)
8002adec:  lw v0,0x2c(s4)
8002adf0:  nop
8002adf4:  subu v1,v1,v0
8002adf8:  bgez v1,0x8002ae0c
8002adfc:  _sra s2,a0,0x11
8002ae00:  lui v0,0x1
8002ae04:  ori v0,v0,0xffff
8002ae08:  addu v1,v1,v0
8002ae0c:  mult s2,s2
8002ae10:  mflo v0
8002ae14:  sra s1,v1,0x11
8002ae18:  nop
8002ae1c:  mult s1,s1
8002ae20:  mflo v1
8002ae24:  addu a0,v0,v1
8002ae28:  slti v0,a0,0x2da
8002ae2c:  bne v0,zero,0x8002ae80
8002ae30:  _nop
8002ae34:  jal 0x8004c6e4
8002ae38:  _nop
8002ae3c:  lui v1,0x1
8002ae40:  ori v1,v1,0xb000
8002ae44:  div v1,v0
8002ae48:  mflo v1
8002ae4c:  nop
8002ae50:  nop
8002ae54:  mult s2,v1
8002ae58:  mflo a0
8002ae5c:  nop
8002ae60:  bgez a0,0x8002ae6c
8002ae64:  _mult s1,v1
8002ae68:  addiu a0,a0,0xfff
8002ae6c:  mflo v1
8002ae70:  bgez v1,0x8002ae7c
8002ae74:  _sra s2,a0,0xc
8002ae78:  addiu v1,v1,0xfff
8002ae7c:  sra s1,v1,0xc
8002ae80:  lw v0,0xe4(s4)
8002ae84:  nop
8002ae88:  beq v0,s0,0x8002aebc
8002ae8c:  _lui a0,0x6000
8002ae90:  lh v0,0x6(s0)
8002ae94:  nop
8002ae98:  bgez v0,0x8002aec0
8002ae9c:  _ori a0,a0,0xff
8002aea0:  lb v1,0x15(gp)
8002aea4:  li v0,0x4
8002aea8:  bne v1,v0,0x8002aec0
8002aeac:  _nop
8002aeb0:  lui a0,0x6080
8002aeb4:  j 0x8002aec0
8002aeb8:  _ori a0,a0,0x8080
8002aebc:  ori a0,a0,0xff00
8002aec0:  sw a0,0x4(s3)
8002aec4:  lh v0,0x10(s4)
8002aec8:  nop
8002aecc:  mult v0,s2
8002aed0:  lh v0,0x1c(s4)
8002aed4:  mflo v1
8002aed8:  nop
8002aedc:  nop
8002aee0:  mult v0,s1
8002aee4:  mflo t1
8002aee8:  addu v0,v1,t1
8002aeec:  bgez v0,0x8002aef8
8002aef0:  _nop
8002aef4:  addiu v0,v0,0xfff
8002aef8:  sra v0,v0,0xc
8002aefc:  addu v0,s8,v0
8002af00:  sh v0,0x8(s3)
8002af04:  lh v0,0x14(s4)
8002af08:  nop
8002af0c:  mult v0,s2
8002af10:  lh v0,0x20(s4)
8002af14:  mflo v1
8002af18:  nop
8002af1c:  nop
8002af20:  mult v0,s1
8002af24:  mflo t1
8002af28:  addu v0,v1,t1
8002af2c:  bgez v0,0x8002af38
8002af30:  _nop
8002af34:  addiu v0,v0,0xfff
8002af38:  lw t0,0x44(sp)
8002af3c:  sra v0,v0,0xc
8002af40:  move a0,s7
8002af44:  move a1,s3
8002af48:  subu v0,t0,v0
8002af4c:  jal 0x80052274
8002af50:  _sh v0,0xa(s3)
8002af54:  move v1,s5
8002af58:  lw s5,0x0(s5)
8002af5c:  nop
8002af60:  bne s5,zero,0x8002ad80
8002af64:  _nop
8002af68:  lw ra,0x34(sp)
8002af6c:  lw s8,0x30(sp)
8002af70:  lw s7,0x2c(sp)
8002af74:  lw s6,0x28(sp)
8002af78:  lw s5,0x24(sp)
8002af7c:  lw s4,0x20(sp)
8002af80:  lw s3,0x1c(sp)
8002af84:  lw s2,0x18(sp)
8002af88:  lw s1,0x14(sp)
8002af8c:  lw s0,0x10(sp)
8002af90:  jr ra
8002af94:  _addiu sp,sp,0x38
