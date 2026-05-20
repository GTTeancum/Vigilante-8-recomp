// addr: 0x801036bc  name: FUN_801036bc

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_801036bc(void)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 *in_v0;
  
  puVar1 = _DAT_80065bd0;
  if ((int)in_v0 < 0x100000) {
    puVar2 = in_v0;
    *_DAT_80065bd0 = in_v0;
    _DAT_80065bd0 = puVar2;
    in_v0[1] = puVar1;
    *in_v0 = 0x80065bcc;
  }
  return;
}

