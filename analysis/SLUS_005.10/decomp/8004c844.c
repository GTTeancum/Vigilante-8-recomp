// addr: 0x8004c844  name: VectorNormalSS

long VectorNormalSS(SVECTOR *_2,SVECTOR *_3)

{
  long lVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  
  sVar2 = _2->vx;
  sVar3 = _2->vy;
  sVar4 = _2->vz;
  lVar1 = MSC02_OBJ_100();
  _3->vx = sVar2;
  _3->vy = sVar3;
  _3->vz = sVar4;
  return lVar1;
}

