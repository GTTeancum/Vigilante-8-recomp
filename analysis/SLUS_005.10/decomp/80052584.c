// addr: 0x80052584  name: strncmp

int strncmp(char *param_1,char *param_2,int param_3)

{
  char cVar1;
  char cVar2;
  int iVar3;
  int iVar4;
  
  if ((param_1 == (char *)0x0) || (param_2 == (char *)0x0)) {
    iVar3 = 0;
    if ((param_1 != param_2) && (iVar3 = -1, param_1 != (char *)0x0)) {
      iVar3 = STRNCMP_OBJ_78();
      return iVar3;
    }
  }
  else {
    iVar4 = param_3 + -1;
    iVar3 = 0;
    if (-1 < iVar4) {
      do {
        cVar1 = *param_1;
        cVar2 = *param_2;
        param_2 = param_2 + 1;
        if (cVar1 != cVar2) break;
        param_1 = param_1 + 1;
        if (cVar1 == '\0') goto STRNCMP_OBJ_74;
        iVar4 = iVar4 + -1;
      } while (-1 < iVar4);
      if (-1 < iVar4) {
        iVar3 = STRNCMP_OBJ_78();
        return iVar3;
      }
STRNCMP_OBJ_74:
      iVar3 = 0;
    }
  }
  return iVar3;
}

