// addr: 0x8004f5e8  name: SYS_OBJ_3E4

void SYS_OBJ_3E4(undefined4 param_1,short *param_2)

{
  int iVar1;
  int iVar2;
  
  if (DAT_80065026 != '\x01') {
    if (DAT_80065026 == '\x02') {
      printf("%s:",param_1);
      printf("(%d,%d)-(%d,%d)\n",(int)*param_2,(int)param_2[1],(int)param_2[2],(int)param_2[3]);
      return;
    }
    SYS_OBJ_4F0();
    return;
  }
  iVar2 = (int)param_2[2];
  if ((iVar2 <= DAT_80065028) && (iVar2 + *param_2 <= (int)DAT_80065028)) {
    iVar1 = (int)param_2[1];
    if (((iVar1 <= DAT_8006502a) &&
        (((iVar1 + param_2[3] <= (int)DAT_8006502a && (0 < iVar2)) && (-1 < *param_2)))) &&
       ((-1 < iVar1 && (0 < param_2[3])))) {
      return;
    }
  }
  SYS_OBJ_4B4("%s:bad RECT");
  return;
}

