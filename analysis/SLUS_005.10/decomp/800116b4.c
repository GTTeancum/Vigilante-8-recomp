// addr: 0x800116b4  name: start

/* WARNING: This function may have set the stack pointer */

void start(void)

{
  bool bVar1;
  undefined4 *puVar2;
  
  puVar2 = &DAT_800658d8;
  do {
    *puVar2 = 0;
    bVar1 = puVar2 < &UNK_800a4f10;
    puVar2 = puVar2 + 1;
  } while (bVar1);
  FUN_80015098();
  return;
}

