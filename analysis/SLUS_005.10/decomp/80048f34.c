// addr: 0x80048f34  name: CdIntstr

char * CdIntstr(u_char intr)

{
  char *pcVar1;
  
  if (intr < 7) {
    pcVar1 = (char *)SYS_OBJ_13C();
    return pcVar1;
  }
  return "none";
}

