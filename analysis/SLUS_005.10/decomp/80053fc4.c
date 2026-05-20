// addr: 0x80053fc4  name: PadSetActAlign

int PadSetActAlign(int param_1,uchar *param_2)

{
  undefined4 uVar1;
  int iVar2;
  
  uVar1 = (*DAT_80065270)();
  iVar2 = _padSetActAlign(uVar1,param_2);
  return iVar2;
}

