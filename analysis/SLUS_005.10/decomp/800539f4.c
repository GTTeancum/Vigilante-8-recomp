// addr: 0x800539f4  name: ReturnFromException

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void ReturnFromException(void)

{
  (*(code *)&SUB_000000b0)();
  return;
}

