// addr: 0x80045374  name: _SpuInit

/* WARNING: Unknown calling convention -- yet parameter storage is locked */

void _SpuInit(void)

{
  undefined2 *puVar1;
  int iVar2;
  int in_a0;
  
  ResetCallback();
  _spu_init();
  if (in_a0 == 0) {
    iVar2 = 0x17;
    puVar1 = &DAT_8005edbe;
    do {
      *puVar1 = 0xc000;
      iVar2 = iVar2 + -1;
      puVar1 = puVar1 + -1;
    } while (-1 < iVar2);
  }
  SpuStart();
  DAT_8005ed68 = 0;
  DAT_8005ed6c = 0;
  DAT_8005ed78 = 0;
  DAT_8005ed7c = 0;
  DAT_8005ed7e = 0;
  DAT_8005ed80 = 0;
  DAT_8005ed84 = 0;
  DAT_8005ed70 = 0xfffe;
  _spu_FsetRXX(0xd1,0xfffe,0);
  DAT_8005ee34 = 0;
  DAT_8005ee38 = 0;
  DAT_8005ee3c = 0;
  DAT_8005ed64 = 0;
  DAT_8005edf0 = 0;
  DAT_8005ed60 = 0;
  DAT_8005ed8c = 0;
  DAT_8005ed88 = 0;
  DAT_8005edc0 = 0;
  return;
}

