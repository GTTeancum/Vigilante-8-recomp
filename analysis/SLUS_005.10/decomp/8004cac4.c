// addr: 0x8004cac4  name: LoadAverageShort12

void LoadAverageShort12(SVECTOR *_2,SVECTOR *_3,long p0,long p1,SVECTOR *v2)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  undefined4 uVar6;
  
  uVar2._0_2_ = _2->vx;
  uVar2._2_2_ = _2->vy;
  uVar4._0_2_ = _2->vz;
  uVar4._2_2_ = _2->pad;
  gte_ldIR0(p0);
  gte_ldsv_(uVar2 & 0xffff,(int)uVar2 >> 0x10,uVar4 & 0xffff);
  gte_gpf12_b();
  uVar1._0_2_ = _3->vx;
  uVar1._2_2_ = _3->vy;
  uVar5._0_2_ = _3->vz;
  uVar5._2_2_ = _3->pad;
  gte_stLZCR();
  gte_ldIR0(p1);
  gte_ldsv_(uVar1 & 0xffff,(int)uVar1 >> 0x10,uVar5 & 0xffff);
  gte_gpl12_b();
  uVar2 = gte_stIR1();
  iVar3 = gte_stIR2();
  uVar2 = uVar2 & 0xffff | iVar3 << 0x10;
  uVar6 = gte_stIR3();
  v2->vx = (short)uVar2;
  v2->vy = (short)(uVar2 >> 0x10);
  v2->vz = (short)uVar6;
  v2->pad = (short)((uint)uVar6 >> 0x10);
  return;
}

