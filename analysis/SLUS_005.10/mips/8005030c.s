# addr: 0x8005030c  name: SetTexWindow
8005030c:  addiu sp,sp,-0x18
80050310:  sw s0,0x10(sp)
80050314:  move s0,a0
80050318:  li v0,0x2
8005031c:  move a0,a1
80050320:  sw ra,0x14(sp)
80050324:  jal 0x80050a7c
80050328:  _sb v0,0x3(s0)
8005032c:  sw v0,0x4(s0)
80050330:  sw zero,0x8(s0)
80050334:  lw ra,0x14(sp)
80050338:  lw s0,0x10(sp)
8005033c:  jr ra
80050340:  _addiu sp,sp,0x18
