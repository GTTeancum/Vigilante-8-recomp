// addr: 0x80048f00  name: CdComstr

char * CdComstr(u_char com)

{
  char *pcVar1;
  
  if (com < 0x1c) {
    pcVar1 = (char *)SYS_OBJ_108();
    return pcVar1;
  }
  return "none";
}

