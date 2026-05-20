// addr: 0x80052604  name: printf

int printf(char *fmt,...)

{
  int iVar1;
  undefined4 in_a1;
  undefined4 in_a2;
  undefined4 in_a3;
  undefined4 local_res4;
  undefined4 local_res8;
  undefined4 local_resc;
  
  local_res4 = in_a1;
  local_res8 = in_a2;
  local_resc = in_a3;
  iVar1 = prnt(1,fmt,&local_res4);
  return iVar1;
}

