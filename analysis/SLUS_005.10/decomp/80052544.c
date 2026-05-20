// addr: 0x80052544  name: FUN_80052544

/* Possible STRLEN.OBJ/strlen */

int FUN_80052544(char *param_1)

{
  char cVar1;
  int iVar2;
  int iVar3;
  
  iVar3 = 0;
  iVar2 = 0;
  if (param_1 != (char *)0x0) {
    while (iVar3 = iVar2, cVar1 = *param_1, param_1 = param_1 + 1, cVar1 != '\0') {
      iVar2 = iVar3 + 1;
    }
  }
  return iVar3;
}

