# addr: 0x8001dcc8  name: FUN_8001dcc8
8001dcc8:  addiu sp,sp,-0x60
8001dccc:  sw s0,0x50(sp)
8001dcd0:  move s0,a0
8001dcd4:  sw s2,0x58(sp)
8001dcd8:  move s2,a1
8001dcdc:  sw ra,0x5c(sp)
8001dce0:  sw s1,0x54(sp)
8001dce4:  addiu s1,sp,0x10
8001dce8:  move a0,s2
8001dcec:  addiu a1,s0,0x10
8001dcf0:  jal 0x8004cf04
8001dcf4:  _move a2,s1
8001dcf8:  lw v1,0x0(s0)
8001dcfc:  nop
8001dd00:  andi v0,v1,0x10
8001dd04:  beq v0,zero,0x8001ddac
8001dd08:  _andi v0,v1,0x400
8001dd0c:  beq v0,zero,0x8001dd60
8001dd10:  _nop
8001dd14:  lw a3,0x10(s0)
8001dd18:  lw t0,0x14(s0)
8001dd1c:  lw t1,0x18(s0)
8001dd20:  lw t2,0x1c(s0)
8001dd24:  sw a3,0x30(sp)
8001dd28:  sw t0,0x34(sp)
8001dd2c:  sw t1,0x38(sp)
8001dd30:  sw t2,0x3c(sp)
8001dd34:  lh a3,0x20(s0)
8001dd38:  nop
8001dd3c:  sh a3,0x40(sp)
8001dd40:  lw a3,0x24(sp)
8001dd44:  lw t0,0x28(sp)
8001dd48:  lw t1,0x2c(sp)
8001dd4c:  sw a3,0x44(sp)
8001dd50:  sw t0,0x48(sp)
8001dd54:  sw t1,0x4c(sp)
8001dd58:  j 0x8001ddac
8001dd5c:  _addiu s1,sp,0x30
8001dd60:  lh v0,0x22(s0)
8001dd64:  nop
8001dd68:  beq v0,zero,0x8001dd80
8001dd6c:  _lui v0,0x8007
8001dd70:  jal 0x80016e64
8001dd74:  _move a0,s1
8001dd78:  j 0x8001ddac
8001dd7c:  _nop
8001dd80:  addiu t2,v0,-0x9a0
8001dd84:  lw a3,0x0(t2)
8001dd88:  lw t0,0x4(t2)
8001dd8c:  lw t1,0x8(t2)
8001dd90:  sw a3,0x10(sp)
8001dd94:  sw t0,0x14(sp)
8001dd98:  sw t1,0x18(sp)
8001dd9c:  lw a3,0xc(t2)
8001dda0:  lh t0,0x10(t2)
8001dda4:  sw a3,0x1c(sp)
8001dda8:  sh t0,0x20(sp)
8001ddac:  lw a0,0x30(s0)
8001ddb0:  nop
8001ddb4:  beq a0,zero,0x8001ddc8
8001ddb8:  _nop
8001ddbc:  lw a2,0x60c(gp)
8001ddc0:  jal 0x8001be5c
8001ddc4:  _move a1,s1
8001ddc8:  lw a0,0x38(s0)
8001ddcc:  nop
8001ddd0:  beq a0,zero,0x8001dde0
8001ddd4:  _nop
8001ddd8:  jal 0x8001dcc8
8001dddc:  _addiu a1,sp,0x10
8001dde0:  lw s0,0x34(s0)
8001dde4:  nop
8001dde8:  bne s0,zero,0x8001dce8
8001ddec:  _addiu s1,sp,0x10
8001ddf0:  lw ra,0x5c(sp)
8001ddf4:  lw s2,0x58(sp)
8001ddf8:  lw s1,0x54(sp)
8001ddfc:  lw s0,0x50(sp)
8001de00:  jr ra
8001de04:  _addiu sp,sp,0x60
