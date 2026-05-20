// addr: 0x8010a704  name: FUN_8010a704

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_8010a704(void)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 local_110 [27];
  undefined4 local_a4;
  undefined4 local_a0;
  undefined4 local_9c;
  undefined4 local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined2 local_8c;
  undefined2 local_8a;
  char local_88;
  undefined1 local_87;
  undefined1 local_86;
  undefined1 local_85;
  
  puVar1 = (undefined4 *)&DAT_80056774;
  puVar2 = local_110;
  do {
    uVar3 = puVar2[1];
    uVar4 = puVar2[2];
    uVar5 = puVar2[3];
    *puVar1 = *puVar2;
    puVar1[1] = uVar3;
    puVar1[2] = uVar4;
    puVar1[3] = uVar5;
    puVar2 = puVar2 + 4;
    puVar1 = puVar1 + 4;
  } while (puVar2 != local_110 + 0x18);
  _DAT_800567d4 = local_110[0x18];
  _DAT_800567d8 = local_110[0x19];
  _DAT_800567dc = local_110[0x1a];
  _DAT_800567e0 = local_a4;
  _DAT_800567e4 = local_a0;
  _DAT_800567e8 = local_9c;
  _DAT_80065950 = local_98;
  _DAT_80065954 = local_94;
  _DAT_80065958 = local_90;
  Audio_SetMasterVolume/*0x80044080*/((int)local_88,local_8c,local_8a,0x200,0x100);
  DAT_8006531a = local_87;
  DAT_8006531c = local_86;
  DAT_8006531d = local_85;
  return;
}

