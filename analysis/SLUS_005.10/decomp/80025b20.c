// addr: 0x80025b20  name: FUN_80025b20

void FUN_80025b20(SVECTOR *param_1,SVECTOR *param_2,SVECTOR *param_3,u_char *param_4,u_char *param_5
                 ,u_char *param_6,int param_7)

{
  int p0;
  
  p0 = 0x1000 - param_7;
  LoadAverageShort12(param_2,param_3,p0,param_7,param_1);
  LoadAverageByte(param_5,param_6,p0,param_7,param_4);
  LoadAverageCol(param_5 + -8,param_6 + -8,p0,param_7,param_4 + -8);
  return;
}

