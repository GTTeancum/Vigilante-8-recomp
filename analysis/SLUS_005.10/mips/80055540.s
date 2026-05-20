# addr: 0x80055540  name: _padCmdParaMode
80055540:  li v0,0x43
80055544:  sb v0,0x36(a0)
80055548:  addiu v0,a0,0x24
8005554c:  sw v0,0x2c(a0)
80055550:  li v0,0x1
80055554:  sb a1,0x24(a0)
80055558:  jr ra
8005555c:  _sb v0,0x35(a0)
