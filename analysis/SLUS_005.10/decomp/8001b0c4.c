// addr: 0x8001b0c4  name: FUN_8001b0c4

undefined4 FUN_8001b0c4(uint *param_1)

{
  ushort uVar1;
  uint uVar2;
  int *piVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  ushort *puVar7;
  
  piVar3 = (int *)param_1[0x16];
  if (piVar3 != (int *)0x0) {
    iVar4 = *piVar3;
    uVar1 = *(ushort *)(iVar4 + (uint)*(ushort *)((int)param_1 + 10) * 0x1c + 0x36);
    while (uVar1 != 0xffff) {
      iVar6 = (uint)uVar1 * 0x1c;
      iVar5 = iVar4 + iVar6;
      if ((*(ushort *)(iVar5 + 0x1c) & 0xf000) == 0xc000) {
        puVar7 = (ushort *)(iVar4 + iVar6 + 0x1c);
        if ((*puVar7 & 0x800) != 0) {
          *param_1 = *param_1 | 0x1000;
        }
        uVar2 = FUN_8001b49c(piVar3,*puVar7 & 0x7ff);
        param_1[0x1a] = uVar2;
        iVar4 = (uint)puVar7[0xb] * 0x10000;
        if (puVar7[0xb] == 0) {
          iVar4 = param_1[0x15] * (int)sRam000005e4;
        }
        iVar4 = iVar4 * (uint)bRam000006cf;
        if (iVar4 < 0) {
          iVar4 = iVar4 + 0xff;
        }
        param_1[0x1b] = iVar4 >> 8;
        return 1;
      }
      uVar1 = *(ushort *)(iVar5 + 0x34);
    }
  }
  param_1[0x1a] = 0;
  param_1[0x1b] = 0;
  return 0;
}

