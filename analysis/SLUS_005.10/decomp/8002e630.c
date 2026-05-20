// addr: 0x8002e630  name: FUN_8002e630

uint * FUN_8002e630(int *param_1,undefined2 param_2,int param_3)

{
  undefined1 uVar1;
  undefined2 uVar2;
  uint *puVar3;
  char cVar4;
  ushort uVar5;
  uint *puVar6;
  uint *puVar7;
  int iVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  int iVar12;
  uint local_30;
  
  puVar6 = (uint *)FUN_8001ac44(param_1,param_2,0x124,(uint)(param_1[1] != 0) << 3);
  *(undefined2 *)((int)puVar6 + 6) = 0;
  *(undefined1 *)(puVar6 + 1) = 2;
  *puVar6 = *puVar6 | 0x6000;
  *(undefined2 *)(puVar6 + 3) = *(undefined2 *)(param_3 + 0x1c);
  uVar1 = *(undefined1 *)(param_3 + 0xd);
  *(undefined2 *)(puVar6 + 0x35) = 0x400;
  *(undefined1 *)(puVar6 + 0x34) = uVar1;
  puVar6[0x37] = *(uint *)(param_3 + 0x10);
  if (param_1[1] != 0) {
    *puVar6 = *puVar6 | 4;
  }
  iVar12 = 0xb;
  puVar7 = puVar6 + 0xb;
  puVar6[0x19] = (uint)&LAB_8002e2bc;
  puVar6[0x36] = -puVar6[0x13];
  do {
    puVar7[0x3b] = 0;
    iVar12 = iVar12 + -1;
    puVar7 = puVar7 + -1;
  } while (-1 < iVar12);
  for (uVar10 = puVar6[0xe]; uVar10 != 0; uVar10 = *(uint *)(uVar10 + 0x34)) {
                    /* WARNING: Read-only address (ram,0x800737d4) is written */
    if (*(ushort *)(uVar10 + 6) < 4) {
      cVar4 = FUN_8003fc94(uVar10);
      *(char *)(uVar10 + 8) = cVar4 + '\x01';
      *(undefined2 *)(uVar10 + 0xc) = *(undefined2 *)(param_3 + 0x1c);
      puVar6[*(short *)(uVar10 + 6) + 0x3b] = uVar10;
    }
  }
                    /* WARNING: Read-only address (ram,0x800737d4) is written */
  uVar10 = 0;
  do {
    uVar11 = 9;
    if ((uRam00000604 & 1) == 0) {
      uVar11 = (uint)*(ushort *)(param_3 + ((int)uVar10 >> 1) * 2);
    }
    puVar7 = (uint *)FUN_8001ac44(DAT_800737d4,uVar11,0x9c,0);
    *(undefined1 *)(puVar7 + 1) = 8;
    iVar8 = *DAT_800737d4;
    iVar12 = FUN_8001affc(param_1,param_2,uVar10 - 0x8000 & 0xffff);
    FUN_8001b2fc(puVar6,iVar12,puVar7);
    puVar6[uVar10 + 0x3f] = (uint)puVar7;
    if (*(ushort *)(iVar12 + 0x1a) == 0xffff) {
      uVar9 = 0;
    }
    else {
      uVar9 = *(uint *)(*param_1 + (uint)*(ushort *)(iVar12 + 0x1a) * 0x1c + 0x24);
    }
    puVar7[0x20] = uVar9;
    puVar7[0x22] = puVar7[0x13];
    puVar7[0x21] = puVar7[0x13];
    iVar12 = param_3 + ((int)uVar10 >> 1) * 2;
    *(undefined2 *)(puVar7 + 0x23) = *(undefined2 *)(iVar12 + 4);
    *(undefined2 *)((int)puVar7 + 0x8e) = *(undefined2 *)(iVar12 + 8);
    iVar8 = *(int *)(iVar8 + uVar11 * 0x1c + 0x24);
    iVar12 = iVar8 * -0x6486;
    puVar7[0x24] = -iVar8;
    if (iVar12 < 0) {
      iVar12 = iVar12 + 0xfff;
    }
    puVar7[0x25] = 0x1000000 / (iVar12 >> 0xc);
    uVar5 = FUN_80017160();
    local_30 = (uint)uVar5;
    uVar11 = (int)puVar7 + 0x43U & 3;
    puVar3 = (uint *)(((int)puVar7 + 0x43U) - uVar11);
    *puVar3 = *puVar3 & -1 << (uVar11 + 1) * 8 | (uint)(uVar5 >> (3 - uVar11) * 8);
    uVar11 = (uint)(puVar7 + 0x10) & 3;
    puVar3 = (uint *)((int)(puVar7 + 0x10) - uVar11);
    *puVar3 = *puVar3 & 0xffffffffU >> (4 - uVar11) * 8 | local_30 << uVar11 * 8;
    *(short *)(puVar7 + 0x11) = (short)((uVar10 & 1) << 0xb);
    uVar11 = uVar10 << 0x13;
    if ((int)uVar10 < 2) {
      uVar11 = uVar11 | 0x20000;
    }
    *puVar7 = *puVar7 | ((int)(uint)*(byte *)(param_3 + 0xc) >> (uVar10 & 0x1f) & 1U) << 0x10 |
                        uVar11;
    FUN_8001d708(puVar7);
    uVar10 = uVar10 + 1;
  } while ((int)uVar10 < 4);
  uVar10 = FUN_8001d470(0x80);
  puVar6[0x3e] = uVar10;
  iVar12 = FUN_8001affc(param_1,param_2,0x8100);
  if (iVar12 == 0) {
    *(undefined4 *)(uVar10 + 0x4c) = 0xffffaaab;
    FUN_8001d708(uVar10);
    FUN_8001d4f0(puVar6,uVar10);
  }
  else {
    FUN_8001b2fc(puVar6,iVar12,uVar10);
  }
  uVar10 = param_3 + 0x17U & 3;
  uVar11 = param_3 + 0x14U & 3;
  uVar11 = (*(int *)((param_3 + 0x17U) - uVar10) << (3 - uVar10) * 8 |
           local_30 & 0xffffffffU >> (uVar10 + 1) * 8) & -1 << (4 - uVar11) * 8 |
           *(uint *)((param_3 + 0x14U) - uVar11) >> uVar11 * 8;
  uVar2 = *(undefined2 *)(param_3 + 0x18);
  uVar10 = (int)puVar6 + 0x9fU & 3;
  puVar7 = (uint *)(((int)puVar6 + 0x9fU) - uVar10);
  *puVar7 = *puVar7 & -1 << (uVar10 + 1) * 8 | uVar11 >> (3 - uVar10) * 8;
  uVar10 = (uint)(puVar6 + 0x27) & 3;
  puVar7 = (uint *)((int)(puVar6 + 0x27) - uVar10);
  *puVar7 = *puVar7 & 0xffffffffU >> (4 - uVar10) * 8 | uVar11 << uVar10 * 8;
  *(undefined2 *)(puVar6 + 0x28) = uVar2;
  *(undefined2 *)((int)puVar6 + 0xa2) = *(undefined2 *)(param_3 + 0x1a);
  FUN_80044efc(puVar6 + 0x24,0,0xc);
  puVar6[0x20] = puVar6[0x24];
  puVar6[0x21] = puVar6[0x25];
  puVar6[0x22] = puVar6[0x26];
  puVar6[0x1e] = 0;
  puVar6[0x1d] = 0;
  FUN_80044efc(puVar6 + 0x29,0,0x1c);
  *(undefined1 *)((int)puVar6 + 0xb2) = 1;
  *(ushort *)(puVar6 + 0x2b) = (ushort)*(byte *)(param_3 + 0xe);
  *(short *)(puVar6 + 0x2a) = (short)*(char *)(param_3 + 0x1e);
  *(short *)((int)puVar6 + 0xaa) = (short)*(char *)(param_3 + 0x1f);
  *(undefined1 *)(puVar6 + 0x2d) = *(undefined1 *)(param_3 + 0xf);
  uVar10 = FUN_8001b270(puVar6);
  puVar6[0x1f] = uVar10;
  return puVar6;
}

