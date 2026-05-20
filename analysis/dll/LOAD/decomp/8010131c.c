// addr: 0x8010131c  name: FUN_8010131c

void FUN_8010131c(int param_1,code *param_2)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  iVar2 = *(int *)(param_1 + 0x18);
  iVar4 = *(int *)(param_1 + 0x1c);
  iVar5 = 0;
  if (0 < *(int *)(param_1 + 0x14)) {
    do {
      iVar3 = iVar2;
      switch(*(byte *)(iVar2 + 3) >> 2 & 0xf) {
      case 4:
      case 5:
      case 7:
      case 8:
        *(undefined1 *)(iVar4 + 0x17) = *(undefined1 *)(iVar4 + 7);
        *(undefined1 *)(iVar4 + 0xf) = *(undefined1 *)(iVar4 + 7);
        (*param_2)(iVar4 + 4,iVar2,*(int *)(param_1 + 8) + (uint)*(ushort *)(iVar2 + 4),
                   *(int *)(param_1 + 0x10) + (uint)*(ushort *)(iVar2 + 10));
        (*param_2)(iVar4 + 0xc,iVar2,*(int *)(param_1 + 8) + (uint)*(ushort *)(iVar2 + 6),
                   *(int *)(param_1 + 0x10) + (uint)*(ushort *)(iVar2 + 0xc));
      case 10:
        iVar3 = iVar2 + (uint)*(ushort *)(iVar2 + 10) * 4;
        iVar4 = iVar4 + (uint)*(ushort *)(iVar2 + 10) * 0x28;
      case 9:
      case 0xb:
        *(undefined1 *)(iVar4 + 0x1f) = *(undefined1 *)(iVar4 + 7);
        *(undefined1 *)(iVar4 + 0x13) = *(undefined1 *)(iVar4 + 7);
        (*param_2)(iVar4 + 4,iVar3,*(int *)(param_1 + 8) + (uint)*(ushort *)(iVar3 + 4),
                   *(int *)(param_1 + 0x10) + (uint)*(ushort *)(iVar3 + 10));
        (*param_2)(iVar4 + 0x10,iVar3,*(int *)(param_1 + 8) + (uint)*(ushort *)(iVar3 + 6),
                   *(int *)(param_1 + 0x10) + (uint)*(ushort *)(iVar3 + 0xc));
        (*param_2)(iVar4 + 0x1c,iVar3,*(int *)(param_1 + 8) + (uint)*(ushort *)(iVar3 + 8),
                   *(int *)(param_1 + 0x10) + (uint)*(ushort *)(iVar3 + 0xe));
      }
      uVar1 = *(byte *)(iVar2 + 3) & 0x3c;
      iVar5 = iVar5 + 1;
      iVar4 = iVar4 + (uint)*(ushort *)(uVar1 + 0x800568fe);
      iVar2 = iVar3 + (uint)*(ushort *)(uVar1 + 0x800568fc);
    } while (iVar5 < *(int *)(param_1 + 0x14));
  }
  if (*(int *)(param_1 + 0x20) != 0) {
    V8_MemCopy/*0x80044c44*/(*(int *)(param_1 + 0x20),*(int *)(param_1 + 0x1c),
                    iVar4 - *(int *)(param_1 + 0x1c));
  }
  return;
}

