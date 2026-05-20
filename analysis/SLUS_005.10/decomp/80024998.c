// addr: 0x80024998  name: FUN_80024998

undefined4 * FUN_80024998(int param_1)

{
  ushort uVar1;
  undefined4 *puVar2;
  int iVar3;
  undefined4 *puVar4;
  int iVar5;
  ushort uVar6;
  
  uVar1 = *(ushort *)(*(int *)(param_1 + 8) + (uint)*(byte *)(param_1 + 0x10) * 2 + 2);
  puVar4 = (undefined4 *)0x0;
  uVar6 = 0xf00;
  if (uVar1 != 0) {
    uVar6 = uVar1;
  }
  iVar5 = (uint)*(ushort *)(param_1 + 0xe) + (1 << (*(byte *)(param_1 + 0x11) & 0x1f));
  if ((uVar6 & 0x100) != 0) {
    for (puVar2 = (undefined4 *)FUN_80024888(param_1,*(ushort *)(param_1 + 0xc) - 1);
        puVar2 != (undefined4 *)0x0;
        puVar2 = (undefined4 *)FUN_80024888(puVar2,*(ushort *)(param_1 + 0xc) - 1)) {
      if (*(short *)(puVar2[2] + (uint)*(byte *)(puVar2 + 4) * 2 + 2) != 0) {
        *puVar2 = puVar4;
        puVar4 = puVar2;
      }
      if (iVar5 <= (int)((uint)*(ushort *)((int)puVar2 + 0xe) +
                        (1 << (*(byte *)((int)puVar2 + 0x11) & 0x1f)))) break;
    }
  }
  if ((uVar6 & 0x200) != 0) {
    for (puVar2 = (undefined4 *)
                  FUN_80024888(param_1,(uint)*(ushort *)(param_1 + 0xc) +
                                       (1 << (*(byte *)(param_1 + 0x11) & 0x1f)),
                               *(undefined2 *)(param_1 + 0xe)); puVar2 != (undefined4 *)0x0;
        puVar2 = (undefined4 *)FUN_80024888(puVar2,*(undefined2 *)(puVar2 + 3))) {
      if (*(short *)(puVar2[2] + (uint)*(byte *)(puVar2 + 4) * 2 + 2) != 0) {
        *puVar2 = puVar4;
        puVar4 = puVar2;
      }
      if (iVar5 <= (int)((uint)*(ushort *)((int)puVar2 + 0xe) +
                        (1 << (*(byte *)((int)puVar2 + 0x11) & 0x1f)))) break;
    }
  }
  iVar5 = (uint)*(ushort *)(param_1 + 0xc) + (1 << (*(byte *)(param_1 + 0x11) & 0x1f));
  if ((uVar6 & 0x800) != 0) {
    for (puVar2 = (undefined4 *)
                  FUN_80024888(param_1,(uint)*(ushort *)(param_1 + 0xc),
                               *(ushort *)(param_1 + 0xe) - 1); puVar2 != (undefined4 *)0x0;
        puVar2 = (undefined4 *)FUN_80024888(puVar2,iVar3,*(ushort *)(param_1 + 0xe) - 1)) {
      if (*(short *)(puVar2[2] + (uint)*(byte *)(puVar2 + 4) * 2 + 2) != 0) {
        *puVar2 = puVar4;
        puVar4 = puVar2;
      }
      iVar3 = (uint)*(ushort *)(puVar2 + 3) + (1 << (*(byte *)((int)puVar2 + 0x11) & 0x1f));
      if (iVar5 <= iVar3) break;
    }
  }
  if ((uVar6 & 0x400) != 0) {
    for (puVar2 = (undefined4 *)
                  FUN_80024888(param_1,*(undefined2 *)(param_1 + 0xc),
                               (uint)*(ushort *)(param_1 + 0xe) +
                               (1 << (*(byte *)(param_1 + 0x11) & 0x1f)));
        puVar2 != (undefined4 *)0x0;
        puVar2 = (undefined4 *)FUN_80024888(puVar2,iVar3,*(undefined2 *)((int)puVar2 + 0xe))) {
      if (*(short *)(puVar2[2] + (uint)*(byte *)(puVar2 + 4) * 2 + 2) != 0) {
        *puVar2 = puVar4;
        puVar4 = puVar2;
      }
      iVar3 = (uint)*(ushort *)(puVar2 + 3) + (1 << (*(byte *)((int)puVar2 + 0x11) & 0x1f));
      if (iVar5 <= iVar3) break;
    }
  }
  if (((uVar6 & 0x1000) != 0) &&
     (iVar5 = *(int *)(param_1 + 8) + 10 + (uint)*(byte *)(param_1 + 0x10) * 2,
     puVar2 = (undefined4 *)
              FUN_80024888(param_1,(uint)*(ushort *)(param_1 + 0xc) + (int)*(char *)(iVar5 + 2),
                           (uint)*(ushort *)(param_1 + 0xe) + (int)*(char *)(iVar5 + 3)),
     *(short *)(puVar2[2] + (uint)*(byte *)(puVar2 + 4) * 2 + 2) != 0)) {
    *puVar2 = puVar4;
    puVar4 = puVar2;
  }
  return puVar4;
}

