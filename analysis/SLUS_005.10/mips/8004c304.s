# addr: 0x8004c304  name: C_011_OBJ_8B0
8004c304:  lui v1,0x8006
8004c308:  lw v1,0x448(v1)
8004c30c:  li v0,0x1325
8004c310:  sw v0,0x0(v1)
8004c314:  lui v1,0x800a
8004c318:  lw v1,0x32d8(v1)
8004c31c:  li v0,0x3
8004c320:  sh v0,0x0(v1)
8004c324:  lui v0,0x800a
8004c328:  lw v0,0x32ac(v0)
8004c32c:  lui v1,0x800a
8004c330:  lw v1,0x32b8(v1)
8004c334:  addiu v0,v0,0x1
8004c338:  lui at,0x800a
8004c33c:  beq v1,zero,0x8004c360
8004c340:  _sw v0,0x32ac(at)
8004c344:  lui v0,0x800a
8004c348:  lw v0,0x32a4(v0)
8004c34c:  nop
8004c350:  beq v0,zero,0x8004c360
8004c354:  _nop
8004c358:  jal 0x8004b7e4
8004c35c:  _nop
