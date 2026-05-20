// addr: 0x800539a4  name: _boot

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _boot(void)

{
  (*(code *)&LAB_000000a0)();
  return;
}

