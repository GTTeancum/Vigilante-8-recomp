// addr: 0x80048540  name: INTR_OBJ_4EC

void INTR_OBJ_4EC(void)

{
  int unaff_s1;
  int unaff_s2;
  word unaff_s3;
  
  if (unaff_s1 == 0) {
    ChangeClearPAD((uint)(unaff_s2 == 0));
    ChangeClearRCnt(3,(uint)(unaff_s2 == 0));
  }
  if (unaff_s1 == 4) {
    ChangeClearRCnt(0,unaff_s2 == 0);
  }
  if (unaff_s1 == 5) {
    ChangeClearRCnt(1,unaff_s2 == 0);
  }
  if (unaff_s1 == 6) {
    ChangeClearRCnt(2,unaff_s2 == 0);
  }
  I_MASK = unaff_s3;
  return;
}

