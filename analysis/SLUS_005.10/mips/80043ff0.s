# addr: 0x80043ff0  name: FUN_80043ff0
80043ff0:  beq a0,zero,0x80044020
80043ff4:  _nor v1,zero,a1
80043ff8:  lw v0,0x5b4(gp)
80043ffc:  lui a0,0x1f80
80044000:  ori a0,a0,0x1c00
80044004:  srl v1,a1,0x10
80044008:  sh a1,0x188(a0)
8004400c:  sh v1,0x18a(a0)
80044010:  or v0,v0,a1
80044014:  sw v0,0x5b4(gp)
80044018:  jr ra
8004401c:  _nop
80044020:  lw v0,0x5b4(gp)
80044024:  nop
80044028:  and v0,v0,v1
8004402c:  sw v0,0x5b4(gp)
80044030:  lhu v1,0x5b4(gp)
80044034:  lui a0,0x1f80
80044038:  ori a0,a0,0x1c00
8004403c:  nor v0,zero,v0
80044040:  srl v0,v0,0x10
80044044:  sh v0,0x18e(a0)
80044048:  nor v1,zero,v1
8004404c:  jr ra
80044050:  _sh v1,0x18c(a0)
