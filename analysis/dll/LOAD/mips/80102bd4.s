# addr: 0x80102bd4  name: FUN_80102bd4
80102bd4:  addiu sp,sp,-0xb8
80102bd8:  sw ra,0xb4(sp)
80102bdc:  sw s8,0xb0(sp)
80102be0:  sw s7,0xac(sp)
80102be4:  sw s6,0xa8(sp)
80102be8:  sw s5,0xa4(sp)
80102bec:  sw s4,0xa0(sp)
80102bf0:  sw s3,0x9c(sp)
80102bf4:  sw s2,0x98(sp)
80102bf8:  sw s1,0x94(sp)
80102bfc:  sw s0,0x90(sp)
80102c00:  sw a0,0xb8(sp)
80102c04:  sw a1,0xbc(sp)
80102c08:  sw a2,0xc0(sp)
80102c0c:  lw t0,0xb8(sp)
80102c10:  nop
80102c14:  lw a1,0x0(t0)
80102c18:  lw v0,0xc(t0)
80102c1c:  lw a0,0x18(t0)
80102c20:  lw a2,0x24(t0)
80102c24:  sll v1,v0,0x1
80102c28:  addu v1,v1,v0
80102c2c:  subu v1,v1,a1
80102c30:  sll v0,a0,0x1
80102c34:  addu v0,v0,a0
80102c38:  subu v1,v1,v0
80102c3c:  addu v0,v1,a2
80102c40:  bgez v0,0x80102c4c
80102c44:  _nop
80102c48:  addiu v0,v0,0xf
80102c4c:  lw t1,0xb8(sp)
80102c50:  sra v0,v0,0x4
80102c54:  sw v0,0x30(sp)
80102c58:  lw a1,0x8(t1)
80102c5c:  lw v0,0x14(t1)
80102c60:  lw a0,0x20(t1)
80102c64:  lw a2,0x2c(t1)
80102c68:  sll v1,v0,0x1
80102c6c:  addu v1,v1,v0
80102c70:  subu v1,v1,a1
80102c74:  sll v0,a0,0x1
80102c78:  addu v0,v0,a0
80102c7c:  subu v1,v1,v0
80102c80:  addu v0,v1,a2
80102c84:  bgez v0,0x80102c90
80102c88:  _nop
80102c8c:  addiu v0,v0,0xf
80102c90:  sra v0,v0,0x4
80102c94:  sw v0,0x34(sp)
80102c98:  lw t2,0xb8(sp)
80102c9c:  nop
80102ca0:  lw v0,0x0(t2)
80102ca4:  lw a0,0xc(t2)
80102ca8:  lw a1,0x18(t2)
80102cac:  sll v1,v0,0x1
80102cb0:  addu v1,v1,v0
80102cb4:  sll v0,a0,0x1
80102cb8:  addu v0,v0,a0
80102cbc:  sll v0,v0,0x1
80102cc0:  subu v1,v1,v0
80102cc4:  sll v0,a1,0x1
80102cc8:  addu v0,v0,a1
80102ccc:  addu v0,v1,v0
80102cd0:  bgez v0,0x80102cdc
80102cd4:  _nop
80102cd8:  addiu v0,v0,0xf
80102cdc:  lw t3,0xb8(sp)
80102ce0:  sra v0,v0,0x4
80102ce4:  sw v0,0x38(sp)
80102ce8:  lw v0,0x8(t3)
80102cec:  lw a0,0x14(t3)
80102cf0:  lw a1,0x20(t3)
80102cf4:  sll v1,v0,0x1
80102cf8:  addu v1,v1,v0
80102cfc:  sll v0,a0,0x1
80102d00:  addu v0,v0,a0
80102d04:  sll v0,v0,0x1
80102d08:  subu v1,v1,v0
80102d0c:  sll v0,a1,0x1
80102d10:  addu v0,v0,a1
80102d14:  addu v0,v1,v0
80102d18:  bgez v0,0x80102d24
80102d1c:  _nop
80102d20:  addiu v0,v0,0xf
80102d24:  sra v0,v0,0x4
80102d28:  sw v0,0x3c(sp)
80102d2c:  lw t4,0xb8(sp)
80102d30:  nop
80102d34:  lw v0,0x0(t4)
80102d38:  lw a0,0xc(t4)
80102d3c:  sll v1,v0,0x1
80102d40:  addu v1,v1,v0
80102d44:  sll v0,a0,0x1
80102d48:  addu v0,v0,a0
80102d4c:  subu v0,v0,v1
80102d50:  bgez v0,0x80102d5c
80102d54:  _nop
80102d58:  addiu v0,v0,0xf
80102d5c:  lw t0,0xb8(sp)
80102d60:  sra v0,v0,0x4
80102d64:  sw v0,0x40(sp)
80102d68:  lw v0,0x8(t0)
80102d6c:  lw a0,0x14(t0)
80102d70:  sll v1,v0,0x1
80102d74:  addu v1,v1,v0
80102d78:  sll v0,a0,0x1
80102d7c:  addu v0,v0,a0
80102d80:  subu v0,v0,v1
80102d84:  bgez v0,0x80102d90
80102d88:  _nop
80102d8c:  addiu v0,v0,0xf
80102d90:  sra v0,v0,0x4
80102d94:  sw v0,0x44(sp)
80102d98:  lw t1,0xb8(sp)
80102d9c:  nop
80102da0:  lw v0,0x0(t1)
80102da4:  nop
80102da8:  sw v0,0x48(sp)
80102dac:  lw v0,0x8(t1)
80102db0:  nop
80102db4:  sw v0,0x4c(sp)
80102db8:  lw t2,0x30(sp)
80102dbc:  lw t3,0x34(sp)
80102dc0:  lw t4,0x38(sp)
80102dc4:  lw t0,0x3c(sp)
80102dc8:  sw t2,0x10(sp)
80102dcc:  sw t3,0x14(sp)
80102dd0:  sw t4,0x18(sp)
80102dd4:  sw t0,0x1c(sp)
80102dd8:  lw t2,0x40(sp)
80102ddc:  lw t3,0x44(sp)
80102de0:  lw t4,0x48(sp)
80102de4:  lw t0,0x4c(sp)
80102de8:  sw t2,0x20(sp)
80102dec:  sw t3,0x24(sp)
80102df0:  sw t4,0x28(sp)
80102df4:  sw t0,0x2c(sp)
80102df8:  lw v1,0x10(sp)
80102dfc:  lw a0,0x14(sp)
80102e00:  sll v0,v1,0x1
80102e04:  addu v0,v0,v1
80102e08:  sw v0,0x50(sp)
80102e0c:  sll v0,a0,0x1
80102e10:  addu v0,v0,a0
80102e14:  sw v0,0x54(sp)
80102e18:  lw v0,0x18(sp)
80102e1c:  lw v1,0x1c(sp)
80102e20:  sll v0,v0,0x1
80102e24:  sll v1,v1,0x1
80102e28:  sw v0,0x58(sp)
80102e2c:  sw v1,0x5c(sp)
80102e30:  lw v0,0x20(sp)
80102e34:  lw v1,0x24(sp)
80102e38:  sw v0,0x60(sp)
80102e3c:  sw v1,0x64(sp)
80102e40:  lw t1,0x50(sp)
80102e44:  lw t2,0x54(sp)
80102e48:  lw t3,0x58(sp)
80102e4c:  lw t4,0x5c(sp)
80102e50:  sw t1,0x30(sp)
80102e54:  sw t2,0x34(sp)
80102e58:  sw t3,0x38(sp)
80102e5c:  sw t4,0x3c(sp)
80102e60:  lw t1,0x60(sp)
80102e64:  lw t2,0x64(sp)
80102e68:  sw t1,0x40(sp)
80102e6c:  sw t2,0x44(sp)
80102e70:  lw t0,0xc0(sp)
80102e74:  nop
80102e78:  andi v0,t0,0x1
80102e7c:  beq v0,zero,0x80102ea8
80102e80:  _addiu s0,sp,0x48
80102e84:  lw t1,0xb8(sp)
80102e88:  nop
80102e8c:  lw a0,0x0(t1)
80102e90:  lw a1,0x4(t1)
80102e94:  jal 0x80025800
80102e98:  _move a2,s0
80102e9c:  move a0,s0
80102ea0:  jal 0x8004c844
80102ea4:  _move a1,a0
80102ea8:  clear s4
80102eac:  sw zero,0x68(sp)
80102eb0:  mult s4,s4
80102eb4:  mflo v1
80102eb8:  bgez v1,0x80102ec4
80102ebc:  _nop
80102ec0:  addiu v1,v1,0xfff
80102ec4:  lw v0,0x30(sp)
80102ec8:  sra a0,v1,0xc
80102ecc:  mult v0,a0
80102ed0:  lw v0,0x38(sp)
80102ed4:  mflo v1
80102ed8:  nop
80102edc:  nop
80102ee0:  mult v0,s4
80102ee4:  mflo t5
80102ee8:  addu v0,v1,t5
80102eec:  bgez v0,0x80102ef8
80102ef0:  _nop
80102ef4:  addiu v0,v0,0xfff
80102ef8:  lw v1,0x40(sp)
80102efc:  sra v0,v0,0xc
80102f00:  addu v1,v0,v1
80102f04:  bgez v1,0x80102f10
80102f08:  _nop
80102f0c:  addiu v1,v1,0xff
80102f10:  lw v0,0x34(sp)
80102f14:  nop
80102f18:  mult v0,a0
80102f1c:  lw v0,0x3c(sp)
80102f20:  mflo a0
80102f24:  nop
80102f28:  nop
80102f2c:  mult v0,s4
80102f30:  mflo t5
80102f34:  addu v0,a0,t5
80102f38:  bgez v0,0x80102f44
80102f3c:  _sra a0,v1,0x8
80102f40:  addiu v0,v0,0xfff
80102f44:  lw v1,0x44(sp)
80102f48:  sra v0,v0,0xc
80102f4c:  addu v0,v0,v1
80102f50:  bgez v0,0x80102f5c
80102f54:  _mult a0,a0
80102f58:  addiu v0,v0,0xff
80102f5c:  mflo v1
80102f60:  sra v0,v0,0x8
80102f64:  nop
80102f68:  mult v0,v0
80102f6c:  mflo t5
80102f70:  jal 0x8004c6e4
80102f74:  _addu a0,v1,t5
80102f78:  lw t0,0xbc(sp)
80102f7c:  nop
80102f80:  lw v1,0x28(t0)
80102f84:  nop
80102f88:  div v1,v0
80102f8c:  mflo v1
80102f90:  lw t0,0x68(sp)
80102f94:  nop
80102f98:  addiu t0,t0,0x1
80102f9c:  addu s4,s4,v1
80102fa0:  slti v0,s4,0x1000
80102fa4:  bne v0,zero,0x80102eb0
80102fa8:  _sw t0,0x68(sp)
80102fac:  lw t1,0xbc(sp)
80102fb0:  nop
80102fb4:  lw v0,0x14(t1)
80102fb8:  move a0,t0
80102fbc:  sll v1,t0,0x1
80102fc0:  addu v0,v0,v1
80102fc4:  sltiu a0,zero,0x90f
80102fc8:  sw v0,0x14(t1)
80102fcc:  lw t2,0xb8(sp)
80102fd0:  nop
80102fd4:  lw v1,0x0(t2)
80102fd8:  lw a0,0x24(t2)
80102fdc:  move s3,v0
80102fe0:  addu v1,v1,a0
80102fe4:  srl v0,v1,0x1f
80102fe8:  addu v1,v1,v0
80102fec:  sra v1,v1,0x1
80102ff0:  sw v1,0xc(s3)
80102ff4:  lw a1,0x8(t2)
80102ff8:  lw v0,0x2c(t2)
80102ffc:  sw zero,0x6c(sp)
80103000:  lw a0,0xc(s3)
80103004:  addu a1,a1,v0
80103008:  srl v0,a1,0x1f
8010300c:  addu a1,a1,v0
80103010:  sra a1,a1,0x1
80103014:  jal 0x80025400
80103018:  _sw a1,0x14(s3)
8010301c:  sw v0,0x10(s3)
80103020:  lw t3,0xbc(sp)
80103024:  nop
80103028:  sw t3,0x8(s3)
8010302c:  lw t4,0x68(sp)
80103030:  nop
80103034:  bltz t4,0x80103674
80103038:  _clear s4
8010303c:  addiu t0,sp,0x48
80103040:  li t1,0x8
80103044:  sw t0,0x74(sp)
80103048:  sw zero,0x78(sp)
8010304c:  sw t1,0x7c(sp)
80103050:  sw s3,0x80(sp)
80103054:  mult s4,s4
80103058:  mflo v0
8010305c:  bgez v0,0x8010306c
80103060:  _sra a3,v0,0xc
80103064:  addiu v0,v0,0xfff
80103068:  sra a3,v0,0xc
8010306c:  mult a3,s4
80103070:  mflo v1
80103074:  bgez v1,0x80103080
80103078:  _nop
8010307c:  addiu v1,v1,0xfff
80103080:  lw v0,0x10(sp)
80103084:  sra a1,v1,0xc
80103088:  mult v0,a1
8010308c:  lw v0,0x18(sp)
80103090:  mflo a0
80103094:  nop
80103098:  nop
8010309c:  mult v0,a3
801030a0:  lw v0,0x20(sp)
801030a4:  mflo v1
801030a8:  nop
801030ac:  nop
801030b0:  mult v0,s4
801030b4:  addu v0,a0,v1
801030b8:  mflo t5
801030bc:  addu a0,v0,t5
801030c0:  bgez a0,0x801030cc
801030c4:  _nop
801030c8:  addiu a0,a0,0xff
801030cc:  lw v0,0x14(sp)
801030d0:  nop
801030d4:  mult v0,a1
801030d8:  lw v0,0x1c(sp)
801030dc:  mflo a2
801030e0:  nop
801030e4:  nop
801030e8:  mult v0,a3
801030ec:  lw v0,0x24(sp)
801030f0:  mflo a1
801030f4:  nop
801030f8:  nop
801030fc:  mult v0,s4
80103100:  lw v1,0x28(sp)
80103104:  addu v0,a2,a1
80103108:  mflo t5
8010310c:  addu a1,v0,t5
80103110:  sra v0,a0,0x8
80103114:  bgez a1,0x80103120
80103118:  _addu s7,v0,v1
8010311c:  addiu a1,a1,0xff
80103120:  lw v0,0x30(sp)
80103124:  nop
80103128:  mult v0,a3
8010312c:  lw v0,0x38(sp)
80103130:  mflo a0
80103134:  nop
80103138:  nop
8010313c:  mult v0,s4
80103140:  lw v1,0x2c(sp)
80103144:  sra v0,a1,0x8
80103148:  mflo t5
8010314c:  addu a0,a0,t5
80103150:  bgez a0,0x8010315c
80103154:  _addu s6,v0,v1
80103158:  addiu a0,a0,0xfff
8010315c:  lw v1,0x40(sp)
80103160:  sra v0,a0,0xc
80103164:  addu v1,v0,v1
80103168:  bgez v1,0x80103174
8010316c:  _nop
80103170:  addiu v1,v1,0xff
80103174:  lw v0,0x34(sp)
80103178:  nop
8010317c:  mult v0,a3
80103180:  lw v0,0x3c(sp)
80103184:  mflo a0
80103188:  nop
8010318c:  nop
80103190:  mult v0,s4
80103194:  mflo t5
80103198:  addu v0,a0,t5
8010319c:  bgez v0,0x801031a8
801031a0:  _sra s1,v1,0x8
801031a4:  addiu v0,v0,0xfff
801031a8:  lw v1,0x44(sp)
801031ac:  sra v0,v0,0xc
801031b0:  addu v0,v0,v1
801031b4:  bgez v0,0x801031c0
801031b8:  _mult s1,s1
801031bc:  addiu v0,v0,0xff
801031c0:  mflo v1
801031c4:  sra s0,v0,0x8
801031c8:  nop
801031cc:  mult s0,s0
801031d0:  mflo t5
801031d4:  jal 0x8004c6e4
801031d8:  _addu a0,v1,t5
801031dc:  lw t0,0xbc(sp)
801031e0:  nop
801031e4:  lw v1,0x24(t0)
801031e8:  nop
801031ec:  mult s0,v1
801031f0:  move s8,v0
801031f4:  mflo s0
801031f8:  srl v0,s0,0x1f
801031fc:  addu s0,s0,v0
80103200:  sra s0,s0,0x1
80103204:  div s0,s8
80103208:  mflo s0
8010320c:  nop
80103210:  nop
80103214:  mult s1,v1
80103218:  mflo v1
8010321c:  srl v0,v1,0x1f
80103220:  addu v1,v1,v0
80103224:  sra v1,v1,0x1
80103228:  div v1,s8
8010322c:  mflo v1
80103230:  lw t0,0xc0(sp)
80103234:  nop
80103238:  andi v0,t0,0x1
8010323c:  subu s2,s7,s0
80103240:  addu s7,s7,s0
80103244:  addu s5,s6,v1
80103248:  beq v0,zero,0x80103414
8010324c:  _subu s6,s6,v1
80103250:  lw t1,0xb8(sp)
80103254:  nop
80103258:  lw v1,0x4(t1)
8010325c:  li v0,0x1000
80103260:  subu v0,v0,s4
80103264:  mult v0,v1
80103268:  lw v0,0x28(t1)
8010326c:  mfhi a1
80103270:  mflo a0
80103274:  nop
80103278:  nop
8010327c:  mult s4,v0
80103280:  clear a2
80103284:  clear a3
80103288:  mfhi t1
8010328c:  mflo t0
80103290:  sw t0,0x88(sp)
80103294:  sw t1,0x8c(sp)
80103298:  addu s0,a0,t0
8010329c:  sltu v0,s0,t0
801032a0:  addu s1,a1,t1
801032a4:  addu s1,s1,v0
801032a8:  move a0,s0
801032ac:  sltiu a0,zero,0x1861
801032b0:  move a1,s1
801032b4:  bgtz v0,0x801032c8
801032b8:  _nop
801032bc:  addiu s0,s0,0xfff
801032c0:  sltiu v0,s0,0xfff
801032c4:  addu s1,s1,v0
801032c8:  lw v0,0xc(s3)
801032cc:  nop
801032d0:  subu v0,s2,v0
801032d4:  sra v0,v0,0x8
801032d8:  sh v0,0x50(sp)
801032dc:  lw a0,0x10(s3)
801032e0:  srl v0,s0,0xc
801032e4:  sll a1,s1,0x14
801032e8:  or v0,v0,a1
801032ec:  sra v1,s1,0xc
801032f0:  move s0,v0
801032f4:  subu a0,s0,a0
801032f8:  sra a0,a0,0x8
801032fc:  sh a0,0x52(sp)
80103300:  lw v0,0x14(s3)
80103304:  lw a0,0x74(sp)
80103308:  lui a1,0x8006
8010330c:  addiu a1,a1,0x5ab0
80103310:  subu v0,s5,v0
80103314:  sra v0,v0,0x8
80103318:  jal 0x80016810
8010331c:  _sh v0,0x54(sp)
80103320:  blez v0,0x8010332c
80103324:  _clear v1
80103328:  move v1,v0
8010332c:  bgez v1,0x80103340
80103330:  _li a0,0x80
80103334:  lui v0,0x1
80103338:  ori v0,v0,0xffff
8010333c:  addu v1,v1,v0
80103340:  sra v0,v1,0x11
80103344:  addiu v1,v0,0x20
80103348:  slt v0,v1,a0
8010334c:  beq v0,zero,0x80103358
80103350:  _nop
80103354:  move a0,v1
80103358:  lw t0,0x80(sp)
8010335c:  sh a0,0x56(sp)
80103360:  lw t1,0x50(sp)
80103364:  lw t2,0x54(sp)
80103368:  sw t1,0x20(t0)
8010336c:  sw t2,0x24(t0)
80103370:  lw v0,0xc(s3)
80103374:  nop
80103378:  subu v0,s7,v0
8010337c:  sra v0,v0,0x8
80103380:  sh v0,0x50(sp)
80103384:  lw v0,0x10(s3)
80103388:  nop
8010338c:  subu v0,s0,v0
80103390:  sra v0,v0,0x8
80103394:  sh v0,0x52(sp)
80103398:  lw v0,0x14(s3)
8010339c:  lw a0,0x74(sp)
801033a0:  lui a1,0x8006
801033a4:  addiu a1,a1,0x5ab0
801033a8:  subu v0,s6,v0
801033ac:  sra v0,v0,0x8
801033b0:  jal 0x80016810
801033b4:  _sh v0,0x54(sp)
801033b8:  lw t0,0x7c(sp)
801033bc:  clear v1
801033c0:  blez v0,0x801033cc
801033c4:  _addu s0,s3,t0
801033c8:  move v1,v0
801033cc:  bgez v1,0x801033e0
801033d0:  _li a0,0x80
801033d4:  lui v0,0x1
801033d8:  ori v0,v0,0xffff
801033dc:  addu v1,v1,v0
801033e0:  sra v0,v1,0x11
801033e4:  addiu v1,v0,0x20
801033e8:  slt v0,v1,a0
801033ec:  beq v0,zero,0x801033f8
801033f0:  _nop
801033f4:  move a0,v1
801033f8:  sh a0,0x56(sp)
801033fc:  lw t1,0x50(sp)
80103400:  lw t2,0x54(sp)
80103404:  sw t1,0x20(s0)
80103408:  sw t2,0x24(s0)
8010340c:  slti a0,zero,0xd6a
80103410:  nop
80103414:  lw v0,0xc(s3)
80103418:  move a0,s2
8010341c:  move a1,s5
80103420:  subu v0,s2,v0
80103424:  sra v0,v0,0x8
80103428:  jal 0x80025400
8010342c:  _sh v0,0x50(sp)
80103430:  lw v1,0x10(s3)
80103434:  nop
80103438:  subu v0,v0,v1
8010343c:  sra v0,v0,0x8
80103440:  sh v0,0x52(sp)
80103444:  lw v0,0x14(s3)
80103448:  move a0,s2
8010344c:  subu v0,s5,v0
80103450:  sra v0,v0,0x8
80103454:  bgez a0,0x80103464
80103458:  _sh v0,0x54(sp)
8010345c:  ori t0,zero,0xffff
80103460:  addu a0,a0,t0
80103464:  move a1,s5
80103468:  bgez a1,0x80103478
8010346c:  _sra a2,a0,0x10
80103470:  ori t1,zero,0xffff
80103474:  addu a1,a1,t1
80103478:  sra a0,a1,0x10
8010347c:  srl v1,a0,0x6
80103480:  sll v1,v1,0x2
80103484:  srl v0,a2,0x6
80103488:  sll v0,v0,0x7
8010348c:  addu v1,v1,v0
80103490:  lui t2,0x8009
80103494:  addiu t2,t2,0x1120
80103498:  addu v1,v1,t2
8010349c:  lw v1,0x80(v1)
801034a0:  andi a0,a0,0x3f
801034a4:  sll a0,a0,0x1
801034a8:  andi v0,a2,0x3f
801034ac:  sll v0,v0,0x7
801034b0:  addu a0,a0,v0
801034b4:  addu v1,v1,a0
801034b8:  lhu v0,0x0(v1)
801034bc:  lw t3,0x80(sp)
801034c0:  srl v0,v0,0xb
801034c4:  sll v0,v0,0x2
801034c8:  sh v0,0x56(sp)
801034cc:  lw t4,0x50(sp)
801034d0:  lw t0,0x54(sp)
801034d4:  sw t4,0x20(t3)
801034d8:  sw t0,0x24(t3)
801034dc:  lw v0,0xc(s3)
801034e0:  move a1,s6
801034e4:  move a0,s7
801034e8:  subu v0,s7,v0
801034ec:  sra v0,v0,0x8
801034f0:  jal 0x80025400
801034f4:  _sh v0,0x50(sp)
801034f8:  lw v1,0x10(s3)
801034fc:  lw t3,0x7c(sp)
80103500:  subu v0,v0,v1
80103504:  sra v0,v0,0x8
80103508:  sh v0,0x52(sp)
8010350c:  lw v0,0x14(s3)
80103510:  move a0,s7
80103514:  addu s0,s3,t3
80103518:  subu v0,s6,v0
8010351c:  sra v0,v0,0x8
80103520:  bgez a0,0x80103530
80103524:  _sh v0,0x54(sp)
80103528:  ori t4,zero,0xffff
8010352c:  addu a0,a0,t4
80103530:  move a1,s6
80103534:  bgez a1,0x80103544
80103538:  _sra a2,a0,0x10
8010353c:  ori t0,zero,0xffff
80103540:  addu a1,a1,t0
80103544:  sra a0,a1,0x10
80103548:  srl v1,a0,0x6
8010354c:  sll v1,v1,0x2
80103550:  srl v0,a2,0x6
80103554:  sll v0,v0,0x7
80103558:  addu v1,v1,v0
8010355c:  lui t1,0x8009
80103560:  addiu t1,t1,0x1120
80103564:  addu v1,v1,t1
80103568:  lw v1,0x80(v1)
8010356c:  andi a0,a0,0x3f
80103570:  sll a0,a0,0x1
80103574:  andi v0,a2,0x3f
80103578:  sll v0,v0,0x7
8010357c:  addu a0,a0,v0
80103580:  addu v1,v1,a0
80103584:  lhu v0,0x0(v1)
80103588:  nop
8010358c:  srl v0,v0,0xb
80103590:  sll v0,v0,0x2
80103594:  sh v0,0x56(sp)
80103598:  lw t2,0x50(sp)
8010359c:  lw t3,0x54(sp)
801035a0:  sw t2,0x20(s0)
801035a4:  sw t3,0x24(s0)
801035a8:  lw t1,0x78(sp)
801035ac:  nop
801035b0:  addiu a0,t1,0x20
801035b4:  jal 0x80016988
801035b8:  _addu a0,s3,a0
801035bc:  lw t2,0x70(sp)
801035c0:  move s0,v0
801035c4:  slt v0,s0,t2
801035c8:  beq v0,zero,0x801035d4
801035cc:  _nop
801035d0:  move s0,t2
801035d4:  lw t3,0x78(sp)
801035d8:  nop
801035dc:  addiu a0,t3,0x28
801035e0:  jal 0x80016988
801035e4:  _addu a0,s3,a0
801035e8:  move a0,v0
801035ec:  slt v0,a0,s0
801035f0:  beq v0,zero,0x801035fc
801035f4:  _nop
801035f8:  move a0,s0
801035fc:  lw t4,0x68(sp)
80103600:  lw t0,0x6c(sp)
80103604:  addiu v0,t4,-0x1
80103608:  bne t0,v0,0x80103618
8010360c:  _sw a0,0x70(sp)
80103610:  slti a0,zero,0xd8e
80103614:  li s4,0x1000
80103618:  lw t1,0xbc(sp)
8010361c:  nop
80103620:  lw v0,0x28(t1)
80103624:  nop
80103628:  div v0,s8
8010362c:  mflo v0
80103630:  nop
80103634:  addu s4,s4,v0
80103638:  lw t0,0x78(sp)
8010363c:  lw t1,0x7c(sp)
80103640:  lw t2,0x80(sp)
80103644:  lw t3,0x6c(sp)
80103648:  lw t4,0x68(sp)
8010364c:  addiu t0,t0,0x10
80103650:  addiu t1,t1,0x10
80103654:  addiu t2,t2,0x10
80103658:  addiu t3,t3,0x1
8010365c:  slt v0,t4,t3
80103660:  sw t0,0x78(sp)
80103664:  sw t1,0x7c(sp)
80103668:  sw t2,0x80(sp)
8010366c:  beq v0,zero,0x80103054
80103670:  _sw t3,0x6c(sp)
80103674:  lw a0,0x70(sp)
80103678:  jal 0x8004c6e4
8010367c:  _nop
80103680:  sll v0,v0,0x8
80103684:  sw v0,0x18(s3)
80103688:  lw ra,0xb4(sp)
8010368c:  lw s8,0xb0(sp)
80103690:  lw s7,0xac(sp)
80103694:  lw s6,0xa8(sp)
80103698:  lw s5,0xa4(sp)
8010369c:  lw s4,0xa0(sp)
801036a0:  move v0,s3
801036a4:  lw s3,0x9c(sp)
801036a8:  lw s2,0x98(sp)
801036ac:  lw s1,0x94(sp)
801036b0:  lw s0,0x90(sp)
801036b4:  jr ra
801036b8:  _addiu sp,sp,0xb8
