// addr: 0x80054044  name: PadSetAct

void PadSetAct(int param_1,uchar *param_2,int param_3)

{
  undefined4 uVar1;
  
  uVar1 = (*DAT_80065270)();
  _padSetAct(uVar1,param_2,param_3);
  return;
}

