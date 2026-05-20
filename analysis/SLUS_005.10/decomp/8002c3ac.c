// addr: 0x8002c3ac  name: FUN_8002c3ac

void FUN_8002c3ac(uint *param_1)

{
  uint uVar1;
  uint *puVar2;
  undefined4 uVar3;
  
  if (((*param_1 & 0x4000000) == 0) && (*(short *)((int)param_1 + 0x11e) == 0)) {
    puVar2 = (uint *)FUN_8001d470(0x9c);
    puVar2[0x19] = (uint)&LAB_8002c210;
    puVar2[0x15] = 0x10000;
    puVar2[0xb] = 0;
    puVar2[10] = 0;
    puVar2[9] = 0;
    *puVar2 = *puVar2 | 0xa4;
    uVar1 = DAT_800737d8;
    *(undefined2 *)((int)puVar2 + 0x96) = 0x22;
    *(undefined2 *)((int)puVar2 + 0x82) = 4;
    *(undefined2 *)(puVar2 + 3) = 2;
    puVar2[0x21] = 0x200;
    puVar2[0x26] = uVar1;
    puVar2[0x22] = 0xfffffa00;
    puVar2[0x23] = 0;
    FUN_8001d544(param_1,puVar2);
    FUN_80020744(puVar2);
    FUN_80020890(puVar2,600);
    *param_1 = *param_1 | 0x4000000;
    uVar3 = FUN_8004410c();
    FUN_8004483c(uVar3,uRam000005f8,0x3c,param_1 + 9);
  }
  return;
}

