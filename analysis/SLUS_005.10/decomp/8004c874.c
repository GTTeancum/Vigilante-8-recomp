// addr: 0x8004c874  name: MSC02_OBJ_100

void MSC02_OBJ_100(void)

{
  int iVar1;
  uint uVar2;
  undefined4 in_t0;
  undefined4 in_t1;
  undefined4 in_t2;
  int in_t3;
  uint uVar3;
  int in_t4;
  int in_t5;
  
  gte_ldsv_(in_t0,in_t1,in_t2);
  gte_sqr0_b(0);
  read_mt(in_t3,in_t4,in_t5);
  iVar1 = in_t3 + in_t4 + in_t5;
  gte_ldLZCS(iVar1);
  uVar2 = gte_stLZCR();
  uVar3 = (uVar2 & 0xfffffffe) - 0x18;
  if ((int)uVar3 < 0) {
    iVar1 = iVar1 >> (0x18 - (uVar2 & 0xfffffffe) & 0x1f);
  }
  else {
    iVar1 = iVar1 << (uVar3 & 0x1f);
  }
  gte_ldIR0((int)*(short *)(&DAT_80060628 + (iVar1 + -0x40) * 2));
  gte_ldsv_(in_t0,in_t1,in_t2);
  gte_gpf0(0);
  read_mt(in_t0,in_t1,in_t2);
  return;
}

