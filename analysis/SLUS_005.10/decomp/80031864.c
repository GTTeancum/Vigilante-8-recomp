// addr: 0x80031864  name: FUN_80031864

uint FUN_80031864(int param_1,undefined4 param_2,uint *param_3)

{
  char cVar1;
  undefined4 *puVar2;
  long lVar3;
  int iVar4;
  undefined2 uVar5;
  uint uVar6;
  int iVar7;
  long local_28 [2];
  long local_20;
  
  switch(param_2) {
  case 0:
    FUN_8003c538(param_1,param_3);
    return 0;
  case 1:
    *(undefined2 *)(param_1 + 0xc) = 0x500;
    break;
  case 4:
    iVar7 = *(ushort *)(param_1 + 0xc) - 0x40;
    iVar4 = 0x500;
    if (0x500 < iVar7) {
      iVar4 = iVar7;
    }
    *(short *)(param_1 + 0xc) = (short)iVar4;
    *(undefined1 *)(param_1 + 5) = 0;
    *(undefined1 *)(param_1 + 8) = 0;
    break;
  case 0xb:
    cVar1 = *(char *)(param_1 + 8) + -1;
    *(char *)(param_1 + 8) = cVar1;
    if (cVar1 != -1) {
      return 0;
    }
    iVar4 = FUN_8001ac44(DAT_800737d8,3,0x80,8);
    puVar2 = (undefined4 *)FUN_80031300(param_3,param_1,4,0x98,iVar4);
    *puVar2 = 0x280;
    uVar5 = 7;
    if ((short)param_3[0x47] != 0) {
      uVar5 = 0xe;
    }
    *(undefined2 *)(puVar2 + 3) = uVar5;
    puVar2[0x19] = &LAB_80031634;
    uVar6 = param_3[0x20];
    if ((int)uVar6 < 0) {
      uVar6 = uVar6 + 0x7f;
    }
    puVar2[0x22] = ((int)uVar6 >> 7) + *(short *)(puVar2 + 5) * 4;
    uVar6 = param_3[0x21];
    if ((int)uVar6 < 0) {
      uVar6 = uVar6 + 0x7f;
    }
    puVar2[0x23] = ((int)uVar6 >> 7) + *(short *)((int)puVar2 + 0x1a) * 4;
    uVar6 = param_3[0x22];
    if ((int)uVar6 < 0) {
      uVar6 = uVar6 + 0x7f;
    }
    puVar2[0x24] = ((int)uVar6 >> 7) + *(short *)(puVar2 + 8) * 4;
    *(undefined2 *)(puVar2 + 0x25) = 0x2d;
    FUN_800202f4(puVar2);
    *(undefined1 **)(iVar4 + 100) = &LAB_8003e80c;
    if ((*param_3 & 4) == 0) {
      FUN_800207c4(iVar4);
    }
    iVar4 = (int)*(char *)(param_1 + 5);
    if (iVar4 == 0) {
      cVar1 = FUN_8004410c();
      *(char *)(param_1 + 5) = cVar1;
      iVar4 = (int)cVar1;
    }
    FUN_800447e8(iVar4,uRam000005f8,0x24,puVar2 + 0x12);
    *(char *)(param_1 + 8) = (char)((ushort)*(undefined2 *)(param_1 + 0xc) >> 8);
    *(short *)(param_1 + 0xc) = *(short *)(param_1 + 0xc) + 0x20;
    break;
  case 0xc:
    FUN_800435c0(param_3 + 4,param_3[0x39] + 0x24,local_28);
    lVar3 = ratan2(local_28[0],local_20);
    iVar4 = (lVar3 << 0x14) >> 0x14;
    if (iVar4 < 0) {
      iVar4 = -iVar4;
    }
    if (0x70 < iVar4) {
      return 0;
    }
    return 0x7cfff < local_20 ^ 1;
  case 0xe:
    return 0x8010;
  }
                    /* WARNING: Read-only address (ram,0x800737d8) is written */
  return 0;
}

