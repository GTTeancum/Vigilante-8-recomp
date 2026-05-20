# addr: 0x801039bc  name: FUN_801039bc
801039bc:  addiu sp,sp,-0x198
801039c0:  sw s6,0x188(sp)
801039c4:  move s6,a0
801039c8:  sw s8,0x190(sp)
801039cc:  clear s8
801039d0:  sw s7,0x18c(sp)
801039d4:  lui s7,0x1
801039d8:  sw s5,0x184(sp)
801039dc:  sw s4,0x180(sp)
801039e0:  ori s4,zero,0x8000
801039e4:  move s5,s8
801039e8:  sw ra,0x194(sp)
801039ec:  sw s3,0x17c(sp)
801039f0:  sw s2,0x178(sp)
801039f4:  sw s1,0x174(sp)
801039f8:  sw s0,0x170(sp)
801039fc:  sw a1,0x19c(sp)
80103a00:  sw a2,0x1a0(sp)
80103a04:  lw v0,0xc(s6)
80103a08:  lui s2,0x1
80103a0c:  clear s3
80103a10:  move a2,v0
80103a14:  sra a3,v0,0x1f
80103a18:  sltu v0,s2,s4
80103a1c:  subu s0,s2,s4
80103a20:  subu s1,s3,s5
80103a24:  subu s1,s1,v0
80103a28:  multu s0,a2
80103a2c:  lw t0,0x18(s6)
80103a30:  mfhi a1
80103a34:  mflo a0
80103a38:  move v0,t0
80103a3c:  sra v1,t0,0x1f
80103a40:  multu s4,v0
80103a44:  mfhi t1
80103a48:  mflo t0
80103a4c:  nop
80103a50:  nop
80103a54:  mult s0,a3
80103a58:  mflo t4
80103a5c:  nop
80103a60:  nop
80103a64:  mult a2,s1
80103a68:  mflo t2
80103a6c:  nop
80103a70:  nop
80103a74:  mult s4,v1
80103a78:  mflo t3
80103a7c:  nop
80103a80:  nop
80103a84:  mult v0,s5
80103a88:  move a2,s2
80103a8c:  move a3,s3
80103a90:  addu a1,a1,t4
80103a94:  addu a1,a1,t2
80103a98:  addu t1,t1,t3
80103a9c:  mflo v0
80103aa0:  addu t1,t1,v0
80103aa4:  addu a0,a0,t0
80103aa8:  sltu v0,a0,t0
80103aac:  addu a1,a1,t1
80103ab0:  jal 0x8004779c
80103ab4:  _addu a1,a1,v0
80103ab8:  sw v0,0x78(sp)
80103abc:  lw v0,0x14(s6)
80103ac0:  nop
80103ac4:  move a2,v0
80103ac8:  sra a3,v0,0x1f
80103acc:  multu s0,a2
80103ad0:  lw t0,0x20(s6)
80103ad4:  mfhi a1
80103ad8:  mflo a0
80103adc:  move v0,t0
80103ae0:  sra v1,t0,0x1f
80103ae4:  multu s4,v0
80103ae8:  mfhi t1
80103aec:  mflo t0
80103af0:  nop
80103af4:  nop
80103af8:  mult s0,a3
80103afc:  mflo t4
80103b00:  nop
80103b04:  nop
80103b08:  mult a2,s1
80103b0c:  mflo t2
80103b10:  nop
80103b14:  nop
80103b18:  mult s4,v1
80103b1c:  mflo t3
80103b20:  nop
80103b24:  nop
80103b28:  mult v0,s5
80103b2c:  move a2,s2
80103b30:  move a3,s3
80103b34:  addu a1,a1,t4
80103b38:  addu a1,a1,t2
80103b3c:  addu t1,t1,t3
80103b40:  mflo v0
80103b44:  addu t1,t1,v0
80103b48:  addu a0,a0,t0
80103b4c:  sltu v0,a0,t0
80103b50:  addu a1,a1,t1
80103b54:  jal 0x8004779c
80103b58:  _addu a1,a1,v0
80103b5c:  sw v0,0x7c(sp)
80103b60:  lw t5,0x78(sp)
80103b64:  lw t7,0x7c(sp)
80103b68:  sw t5,0x70(sp)
80103b6c:  sw t7,0x74(sp)
80103b70:  lw t5,0x0(s6)
80103b74:  lw t7,0x4(s6)
80103b78:  lw t8,0x8(s6)
80103b7c:  sw t5,0x10(sp)
80103b80:  sw t7,0x14(sp)
80103b84:  sw t8,0x18(sp)
80103b88:  lw t5,0x24(s6)
80103b8c:  lw t7,0x28(s6)
80103b90:  lw t8,0x2c(s6)
80103b94:  sw t5,0x64(sp)
80103b98:  sw t7,0x68(sp)
80103b9c:  sw t8,0x6c(sp)
80103ba0:  lw v0,0x0(s6)
80103ba4:  nop
80103ba8:  move a2,v0
80103bac:  sra a3,v0,0x1f
80103bb0:  multu s0,a2
80103bb4:  lw t0,0xc(s6)
80103bb8:  mfhi a1
80103bbc:  mflo a0
80103bc0:  move v0,t0
80103bc4:  sra v1,t0,0x1f
80103bc8:  multu s4,v0
80103bcc:  mfhi t1
80103bd0:  mflo t0
80103bd4:  nop
80103bd8:  nop
80103bdc:  mult s0,a3
80103be0:  mflo t4
80103be4:  nop
80103be8:  nop
80103bec:  mult a2,s1
80103bf0:  mflo t2
80103bf4:  nop
80103bf8:  nop
80103bfc:  mult s4,v1
80103c00:  mflo t3
80103c04:  nop
80103c08:  nop
80103c0c:  mult v0,s5
80103c10:  move a2,s2
80103c14:  move a3,s3
80103c18:  addu a1,a1,t4
80103c1c:  addu a1,a1,t2
80103c20:  addu t1,t1,t3
80103c24:  mflo v0
80103c28:  addu t1,t1,v0
80103c2c:  addu a0,a0,t0
80103c30:  sltu v0,a0,t0
80103c34:  addu a1,a1,t1
80103c38:  jal 0x8004779c
80103c3c:  _addu a1,a1,v0
80103c40:  sw v0,0x80(sp)
80103c44:  sw zero,0x84(sp)
80103c48:  lw v0,0x8(s6)
80103c4c:  nop
80103c50:  move a2,v0
80103c54:  sra a3,v0,0x1f
80103c58:  multu s0,a2
80103c5c:  lw t0,0x14(s6)
80103c60:  mfhi a1
80103c64:  mflo a0
80103c68:  move v0,t0
80103c6c:  sra v1,t0,0x1f
80103c70:  multu s4,v0
80103c74:  mfhi t1
80103c78:  mflo t0
80103c7c:  nop
80103c80:  nop
80103c84:  mult s0,a3
80103c88:  mflo t4
80103c8c:  nop
80103c90:  nop
80103c94:  mult a2,s1
80103c98:  mflo t2
80103c9c:  nop
80103ca0:  nop
80103ca4:  mult s4,v1
80103ca8:  mflo t3
80103cac:  nop
80103cb0:  nop
80103cb4:  mult v0,s5
80103cb8:  move a2,s2
80103cbc:  move a3,s3
80103cc0:  addu a1,a1,t4
80103cc4:  addu a1,a1,t2
80103cc8:  addu t1,t1,t3
80103ccc:  mflo v0
80103cd0:  addu t1,t1,v0
80103cd4:  addu a0,a0,t0
80103cd8:  sltu v0,a0,t0
80103cdc:  addu a1,a1,t1
80103ce0:  jal 0x8004779c
80103ce4:  _addu a1,a1,v0
80103ce8:  sw v0,0x88(sp)
80103cec:  lw t5,0x80(sp)
80103cf0:  lw t7,0x84(sp)
80103cf4:  lw t8,0x88(sp)
80103cf8:  sw t5,0x1c(sp)
80103cfc:  sw t7,0x20(sp)
80103d00:  sw t8,0x24(sp)
80103d04:  lw v0,0x1c(sp)
80103d08:  nop
80103d0c:  move a2,v0
80103d10:  sra a3,v0,0x1f
80103d14:  multu s0,a2
80103d18:  lw t0,0x70(sp)
80103d1c:  mfhi a1
80103d20:  mflo a0
80103d24:  move v0,t0
80103d28:  sra v1,t0,0x1f
80103d2c:  multu s4,v0
80103d30:  mfhi t1
80103d34:  mflo t0
80103d38:  nop
80103d3c:  nop
80103d40:  mult s0,a3
80103d44:  mflo t4
80103d48:  nop
80103d4c:  nop
80103d50:  mult a2,s1
80103d54:  mflo t2
80103d58:  nop
80103d5c:  nop
80103d60:  mult s4,v1
80103d64:  mflo t3
80103d68:  nop
80103d6c:  nop
80103d70:  mult v0,s5
80103d74:  move a2,s2
80103d78:  move a3,s3
80103d7c:  addu a1,a1,t4
80103d80:  addu a1,a1,t2
80103d84:  addu t1,t1,t3
80103d88:  mflo v0
80103d8c:  addu t1,t1,v0
80103d90:  addu a0,a0,t0
80103d94:  sltu v0,a0,t0
80103d98:  addu a1,a1,t1
80103d9c:  jal 0x8004779c
80103da0:  _addu a1,a1,v0
80103da4:  lw a0,0x24(sp)
80103da8:  nop
80103dac:  move t0,a0
80103db0:  sra t1,a0,0x1f
80103db4:  multu s0,t0
80103db8:  lw t2,0x74(sp)
80103dbc:  mfhi a1
80103dc0:  mflo a0
80103dc4:  move a2,t2
80103dc8:  sra a3,t2,0x1f
80103dcc:  multu s4,a2
80103dd0:  mfhi t3
80103dd4:  mflo t2
80103dd8:  nop
80103ddc:  nop
80103de0:  mult s0,t1
80103de4:  mflo t6
80103de8:  nop
80103dec:  nop
80103df0:  mult t0,s1
80103df4:  mflo t0
80103df8:  nop
80103dfc:  nop
80103e00:  mult s4,a3
80103e04:  mflo t1
80103e08:  nop
80103e0c:  nop
80103e10:  mult a2,s5
80103e14:  sw v0,0x78(sp)
80103e18:  sw zero,0x7c(sp)
80103e1c:  addu a1,a1,t6
80103e20:  addu a1,a1,t0
80103e24:  move a2,s2
80103e28:  move a3,s3
80103e2c:  addu t3,t3,t1
80103e30:  mflo t4
80103e34:  addu t3,t3,t4
80103e38:  addu a0,a0,t2
80103e3c:  sltu v0,a0,t2
80103e40:  addu a1,a1,t3
80103e44:  jal 0x8004779c
80103e48:  _addu a1,a1,v0
80103e4c:  sw v0,0x80(sp)
80103e50:  lw t5,0x78(sp)
80103e54:  lw t7,0x7c(sp)
80103e58:  lw t8,0x80(sp)
80103e5c:  sw t5,0x28(sp)
80103e60:  sw t7,0x2c(sp)
80103e64:  sw t8,0x30(sp)
80103e68:  lw v0,0x24(s6)
80103e6c:  nop
80103e70:  move a2,v0
80103e74:  sra a3,v0,0x1f
80103e78:  multu s4,a2
80103e7c:  lw t0,0x18(s6)
80103e80:  mfhi a1
80103e84:  mflo a0
80103e88:  move v0,t0
80103e8c:  sra v1,t0,0x1f
80103e90:  multu s0,v0
80103e94:  mfhi t1
80103e98:  mflo t0
80103e9c:  nop
80103ea0:  nop
80103ea4:  mult s4,a3
80103ea8:  mflo t4
80103eac:  nop
80103eb0:  nop
80103eb4:  mult a2,s5
80103eb8:  mflo t2
80103ebc:  nop
80103ec0:  nop
80103ec4:  mult s0,v1
80103ec8:  mflo t3
80103ecc:  nop
80103ed0:  nop
80103ed4:  mult v0,s1
80103ed8:  move a2,s2
80103edc:  move a3,s3
80103ee0:  addu a1,a1,t4
80103ee4:  addu a1,a1,t2
80103ee8:  addu t1,t1,t3
80103eec:  mflo v0
80103ef0:  addu t1,t1,v0
80103ef4:  addu a0,a0,t0
80103ef8:  sltu v0,a0,t0
80103efc:  addu a1,a1,t1
80103f00:  jal 0x8004779c
80103f04:  _addu a1,a1,v0
80103f08:  sw v0,0x78(sp)
80103f0c:  sw zero,0x7c(sp)
80103f10:  lw v0,0x2c(s6)
80103f14:  nop
80103f18:  move a2,v0
80103f1c:  sra a3,v0,0x1f
80103f20:  multu s4,a2
80103f24:  lw t0,0x20(s6)
80103f28:  mfhi a1
80103f2c:  mflo a0
80103f30:  move v0,t0
80103f34:  sra v1,t0,0x1f
80103f38:  multu s0,v0
80103f3c:  mfhi t1
80103f40:  mflo t0
80103f44:  nop
80103f48:  nop
80103f4c:  mult s4,a3
80103f50:  mflo t4
80103f54:  nop
80103f58:  nop
80103f5c:  mult a2,s5
80103f60:  mflo t2
80103f64:  nop
80103f68:  nop
80103f6c:  mult s0,v1
80103f70:  mflo t3
80103f74:  nop
80103f78:  nop
80103f7c:  mult v0,s1
80103f80:  move a2,s2
80103f84:  move a3,s3
80103f88:  addu a1,a1,t4
80103f8c:  addu a1,a1,t2
80103f90:  addu t1,t1,t3
80103f94:  mflo v0
80103f98:  addu t1,t1,v0
80103f9c:  addu a0,a0,t0
80103fa0:  sltu v0,a0,t0
80103fa4:  addu a1,a1,t1
80103fa8:  jal 0x8004779c
80103fac:  _addu a1,a1,v0
80103fb0:  sw v0,0x80(sp)
80103fb4:  lw t5,0x78(sp)
80103fb8:  lw t7,0x7c(sp)
80103fbc:  lw t8,0x80(sp)
80103fc0:  sw t5,0x58(sp)
80103fc4:  sw t7,0x5c(sp)
80103fc8:  sw t8,0x60(sp)
80103fcc:  lw v0,0x58(sp)
80103fd0:  nop
80103fd4:  move a2,v0
80103fd8:  sra a3,v0,0x1f
80103fdc:  multu s4,a2
80103fe0:  lw t0,0x70(sp)
80103fe4:  mfhi a1
80103fe8:  mflo a0
80103fec:  move v0,t0
80103ff0:  sra v1,t0,0x1f
80103ff4:  multu s0,v0
80103ff8:  mfhi t1
80103ffc:  mflo t0
80104000:  nop
80104004:  nop
80104008:  mult s4,a3
8010400c:  mflo t4
80104010:  nop
80104014:  nop
80104018:  mult a2,s5
8010401c:  mflo t2
80104020:  nop
80104024:  nop
80104028:  mult s0,v1
8010402c:  mflo t3
80104030:  nop
80104034:  nop
80104038:  mult v0,s1
8010403c:  move a2,s2
80104040:  move a3,s3
80104044:  addu a1,a1,t4
80104048:  addu a1,a1,t2
8010404c:  addu t1,t1,t3
80104050:  mflo v0
80104054:  addu t1,t1,v0
80104058:  addu a0,a0,t0
8010405c:  sltu v0,a0,t0
80104060:  addu a1,a1,t1
80104064:  jal 0x8004779c
80104068:  _addu a1,a1,v0
8010406c:  lw a0,0x60(sp)
80104070:  nop
80104074:  move t0,a0
80104078:  sra t1,a0,0x1f
8010407c:  multu s4,t0
80104080:  lw t2,0x74(sp)
80104084:  mfhi a1
80104088:  mflo a0
8010408c:  move a2,t2
80104090:  sra a3,t2,0x1f
80104094:  multu s0,a2
80104098:  mfhi t3
8010409c:  mflo t2
801040a0:  nop
801040a4:  nop
801040a8:  mult s4,t1
801040ac:  mflo t6
801040b0:  nop
801040b4:  nop
801040b8:  mult t0,s5
801040bc:  mflo t0
801040c0:  nop
801040c4:  nop
801040c8:  mult s0,a3
801040cc:  mflo t1
801040d0:  nop
801040d4:  nop
801040d8:  mult a2,s1
801040dc:  sw v0,0x78(sp)
801040e0:  sw zero,0x7c(sp)
801040e4:  addu a1,a1,t6
801040e8:  addu a1,a1,t0
801040ec:  move a2,s2
801040f0:  move a3,s3
801040f4:  addu t3,t3,t1
801040f8:  mflo t4
801040fc:  addu t3,t3,t4
80104100:  addu a0,a0,t2
80104104:  sltu v0,a0,t2
80104108:  addu a1,a1,t3
8010410c:  jal 0x8004779c
80104110:  _addu a1,a1,v0
80104114:  sw v0,0x80(sp)
80104118:  lw t5,0x78(sp)
8010411c:  lw t7,0x7c(sp)
80104120:  lw t8,0x80(sp)
80104124:  sw t5,0x4c(sp)
80104128:  sw t7,0x50(sp)
8010412c:  sw t8,0x54(sp)
80104130:  lw v0,0x28(sp)
80104134:  nop
80104138:  move a2,v0
8010413c:  sra a3,v0,0x1f
80104140:  multu s0,a2
80104144:  lw t0,0x4c(sp)
80104148:  mfhi a1
8010414c:  mflo a0
80104150:  move v0,t0
80104154:  sra v1,t0,0x1f
80104158:  multu s4,v0
8010415c:  mfhi t1
80104160:  mflo t0
80104164:  nop
80104168:  nop
8010416c:  mult s0,a3
80104170:  mflo t4
80104174:  nop
80104178:  nop
8010417c:  mult a2,s1
80104180:  mflo t2
80104184:  nop
80104188:  nop
8010418c:  mult s4,v1
80104190:  mflo t3
80104194:  nop
80104198:  nop
8010419c:  mult v0,s5
801041a0:  move a2,s2
801041a4:  move a3,s3
801041a8:  addu a1,a1,t4
801041ac:  addu a1,a1,t2
801041b0:  addu t1,t1,t3
801041b4:  mflo v0
801041b8:  addu t1,t1,v0
801041bc:  addu a0,a0,t0
801041c0:  sltu v0,a0,t0
801041c4:  addu a1,a1,t1
801041c8:  jal 0x8004779c
801041cc:  _addu a1,a1,v0
801041d0:  sw v0,0x78(sp)
801041d4:  lw v0,0x4(s6)
801041d8:  nop
801041dc:  move a2,v0
801041e0:  sra a3,v0,0x1f
801041e4:  multu s0,a2
801041e8:  lw t0,0x28(s6)
801041ec:  mfhi a1
801041f0:  mflo a0
801041f4:  move v0,t0
801041f8:  sra v1,t0,0x1f
801041fc:  multu s4,v0
80104200:  mfhi t1
80104204:  mflo t0
80104208:  nop
8010420c:  nop
80104210:  mult s0,a3
80104214:  mflo t4
80104218:  nop
8010421c:  nop
80104220:  mult a2,s1
80104224:  mflo t2
80104228:  nop
8010422c:  nop
80104230:  mult s4,v1
80104234:  mflo t3
80104238:  nop
8010423c:  nop
80104240:  mult v0,s5
80104244:  move a2,s2
80104248:  move a3,s3
8010424c:  addu a1,a1,t4
80104250:  addu a1,a1,t2
80104254:  addu t1,t1,t3
80104258:  mflo v0
8010425c:  addu t1,t1,v0
80104260:  addu a0,a0,t0
80104264:  sltu v0,a0,t0
80104268:  addu a1,a1,t1
8010426c:  jal 0x8004779c
80104270:  _addu a1,a1,v0
80104274:  lw a0,0x30(sp)
80104278:  nop
8010427c:  move a2,a0
80104280:  sra a3,a0,0x1f
80104284:  multu s0,a2
80104288:  lw t2,0x54(sp)
8010428c:  mfhi a1
80104290:  mflo a0
80104294:  move t0,t2
80104298:  sra t1,t2,0x1f
8010429c:  multu s4,t0
801042a0:  mfhi t3
801042a4:  mflo t2
801042a8:  nop
801042ac:  nop
801042b0:  mult s0,a3
801042b4:  mflo t4
801042b8:  nop
801042bc:  nop
801042c0:  mult a2,s1
801042c4:  mflo s0
801042c8:  nop
801042cc:  nop
801042d0:  mult s4,t1
801042d4:  mflo t6
801042d8:  nop
801042dc:  nop
801042e0:  mult t0,s5
801042e4:  sw v0,0x7c(sp)
801042e8:  move a2,s2
801042ec:  move a3,s3
801042f0:  addu a1,a1,t4
801042f4:  addu a1,a1,s0
801042f8:  addu t3,t3,t6
801042fc:  mflo t0
80104300:  addu t3,t3,t0
80104304:  addu a0,a0,t2
80104308:  sltu v0,a0,t2
8010430c:  addu a1,a1,t3
80104310:  jal 0x8004779c
80104314:  _addu a1,a1,v0
80104318:  sw v0,0x80(sp)
8010431c:  lw t5,0x78(sp)
80104320:  lw t7,0x7c(sp)
80104324:  lw t8,0x80(sp)
80104328:  sw t5,0x40(sp)
8010432c:  sw t7,0x44(sp)
80104330:  sw t8,0x48(sp)
80104334:  lw t5,0x40(sp)
80104338:  lw t7,0x44(sp)
8010433c:  lw t8,0x48(sp)
80104340:  sw t5,0x34(sp)
80104344:  sw t7,0x38(sp)
80104348:  sw t8,0x3c(sp)
8010434c:  lw t5,0x19c(sp)
80104350:  nop
80104354:  beq t5,zero,0x801043e8
80104358:  _nop
8010435c:  lw v1,0x34(sp)
80104360:  lw v0,0x24(s6)
80104364:  nop
80104368:  subu v1,v1,v0
8010436c:  mult v1,v1
80104370:  lw a0,0x3c(sp)
80104374:  lw v0,0x2c(s6)
80104378:  mfhi a3
8010437c:  mflo a2
80104380:  subu a0,a0,v0
80104384:  nop
80104388:  mult a0,a0
8010438c:  lw t5,0x1a0(sp)
80104390:  mfhi t9
80104394:  mflo t8
80104398:  nop
8010439c:  nop
801043a0:  mult t5,t5
801043a4:  sw t8,0x168(sp)
801043a8:  sw t9,0x16c(sp)
801043ac:  addu a2,a2,t8
801043b0:  sltu v0,a2,t8
801043b4:  addu a3,a3,t9
801043b8:  addu a3,a3,v0
801043bc:  mfhi a1
801043c0:  mflo a0
801043c4:  slt v0,a1,a3
801043c8:  bne v0,zero,0x80104474
801043cc:  _move t7,t5
801043d0:  bne a3,a1,0x8010446c
801043d4:  _sltu v0,a0,a2
801043d8:  bne v0,zero,0x80104474
801043dc:  _nop
801043e0:  slti a0,zero,0x111e
801043e4:  move s7,s4
801043e8:  lw v1,0x34(sp)
801043ec:  lw v0,0x0(s6)
801043f0:  nop
801043f4:  subu v1,v1,v0
801043f8:  mult v1,v1
801043fc:  lw a0,0x3c(sp)
80104400:  lw v0,0x8(s6)
80104404:  mfhi a3
80104408:  mflo a2
8010440c:  subu a0,a0,v0
80104410:  nop
80104414:  mult a0,a0
80104418:  lw t5,0x1a0(sp)
8010441c:  mfhi t9
80104420:  mflo t8
80104424:  nop
80104428:  nop
8010442c:  mult t5,t5
80104430:  sw t8,0x168(sp)
80104434:  sw t9,0x16c(sp)
80104438:  addu a2,a2,t8
8010443c:  sltu v0,a2,t8
80104440:  addu a3,a3,t9
80104444:  addu a3,a3,v0
80104448:  mfhi a1
8010444c:  mflo a0
80104450:  slt v0,a1,a3
80104454:  bne v0,zero,0x8010446c
80104458:  _move t7,t5
8010445c:  bne a3,a1,0x80104474
80104460:  _sltu v0,a0,a2
80104464:  beq v0,zero,0x80104474
80104468:  _nop
8010446c:  slti a0,zero,0x111e
80104470:  move s7,s4
80104474:  move s8,s4
80104478:  addu v1,s8,s7
8010447c:  srl v0,v1,0x1f
80104480:  addu v1,v1,v0
80104484:  sra v0,v1,0x1
80104488:  move s4,v0
8010448c:  subu v0,s7,s8
80104490:  slti v0,v0,0x2
80104494:  beq v0,zero,0x80103a04
80104498:  _sra s5,v1,0x1f
8010449c:  lw t8,0x19c(sp)
801044a0:  nop
801044a4:  beq t8,zero,0x801044e8
801044a8:  _addiu v1,sp,0x10
801044ac:  move a0,s6
801044b0:  addiu v0,sp,0x40
801044b4:  lw t9,0x0(v1)
801044b8:  lw t5,0x4(v1)
801044bc:  lw t7,0x8(v1)
801044c0:  lw t8,0xc(v1)
801044c4:  sw t9,0x0(a0)
801044c8:  sw t5,0x4(a0)
801044cc:  sw t7,0x8(a0)
801044d0:  sw t8,0xc(a0)
801044d4:  addiu v1,v1,0x10
801044d8:  bne v1,v0,0x801044b4
801044dc:  _addiu a0,a0,0x10
801044e0:  slti a0,zero,0x1148
801044e4:  nop
801044e8:  move a0,s6
801044ec:  addiu v1,sp,0x40
801044f0:  addiu v0,sp,0x70
801044f4:  lw t9,0x0(v1)
801044f8:  lw t5,0x4(v1)
801044fc:  lw t7,0x8(v1)
80104500:  lw t8,0xc(v1)
80104504:  sw t9,0x0(a0)
80104508:  sw t5,0x4(a0)
8010450c:  sw t7,0x8(a0)
80104510:  sw t8,0xc(a0)
80104514:  addiu v1,v1,0x10
80104518:  bne v1,v0,0x801044f4
8010451c:  _addiu a0,a0,0x10
80104520:  lw ra,0x194(sp)
80104524:  lw s8,0x190(sp)
80104528:  lw s7,0x18c(sp)
8010452c:  lw s6,0x188(sp)
80104530:  lw s5,0x184(sp)
80104534:  lw s4,0x180(sp)
80104538:  lw s3,0x17c(sp)
8010453c:  lw s2,0x178(sp)
80104540:  lw s1,0x174(sp)
80104544:  lw s0,0x170(sp)
80104548:  jr ra
8010454c:  _addiu sp,sp,0x198
