// addr: 0x80052484  name: strcmp

int strcmp(char *param_1,char *param_2)

{
  char cVar1;
  int iVar2;
  
  if ((param_1 == (char *)0x0) || (param_2 == (char *)0x0)) {
    iVar2 = 0;
    if ((param_1 != param_2) && (iVar2 = -1, param_1 != (char *)0x0)) {
      iVar2 = STRCMP_OBJ_5C();
      return iVar2;
    }
  }
  else {
    while( true ) {
      cVar1 = *param_1;
      if (*param_1 != *param_2) break;
      param_1 = param_1 + 1;
      param_2 = param_2 + 1;
      if (cVar1 == '\0') {
        iVar2 = STRCMP_OBJ_5C();
        return iVar2;
      }
    }
    iVar2 = (int)*param_1 - (int)*param_2;
  }
  return iVar2;
}

