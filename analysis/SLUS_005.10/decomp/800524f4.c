// addr: 0x800524f4  name: strcpy

char * strcpy(char *param_1,char *param_2)

{
  char cVar1;
  char *pcVar2;
  char *pcVar3;
  
  pcVar2 = (char *)0x0;
  if ((param_1 != (char *)0x0) && (param_2 != (char *)0x0)) {
    cVar1 = *param_2;
    *param_1 = cVar1;
    pcVar3 = param_1;
    while (pcVar2 = param_1, cVar1 != '\0') {
      param_2 = param_2 + 1;
      pcVar3 = pcVar3 + 1;
      cVar1 = *param_2;
      *pcVar3 = cVar1;
    }
  }
  return pcVar2;
}

