// addr: 0x800481d0  name: SetIntrMask

word SetIntrMask(word param_1)

{
  word wVar1;
  
  wVar1 = I_MASK;
  I_MASK = param_1;
  return wVar1;
}

