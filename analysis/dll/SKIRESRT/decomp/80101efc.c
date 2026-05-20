// addr: 0x80101efc  name: FUN_80101efc

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80101efc(uint *param_1,int param_2,int param_3)

{
  undefined1 uVar1;
  uint *puVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  
  if (param_2 != 1) {
    if (param_2 != 9) {
      return 0;
    }
    if (param_3 != *(short *)((int)param_1 + 6)) {
      return 0;
    }
    if (0x17 < *(int *)(_DAT_800659fc + 0x94)) {
      return 0;
    }
    puVar2 = (uint *)FUN_8001ac44/*0x8001ac44*/(param_1[0x16],*(undefined2 *)((int)param_1 + 10),0x98,0);
    FUN_8001dc1c/*0x8001dc1c*/(puVar2);
    iVar3 = puVar2[0x15] * 0x93c;
    if (iVar3 < 0) {
      iVar3 = iVar3 + 0xfff;
    }
    puVar2[0x15] = iVar3 >> 0xc;
    *(undefined2 *)((int)puVar2 + 6) = 1000;
    uVar1 = FUN_80017160/*0x80017160*/();
    *(undefined1 *)((int)puVar2 + 9) = uVar1;
    *(undefined2 *)(puVar2 + 3) = *(undefined2 *)((int)param_1 + 0xe);
    *puVar2 = *puVar2 | 0x180;
    uVar4 = param_1[0x13];
    uVar5 = param_1[0x14];
    puVar2[0x12] = param_1[0x12];
    puVar2[0x13] = uVar4;
    puVar2[0x14] = uVar5;
    puVar2[0x19] = (uint)FUN_80101a94;
    puVar2[0x21] = 0xfffff415;
    iVar3 = puVar2[0x15] * 0x3243;
    if (iVar3 < 0) {
      iVar3 = iVar3 + 0xfff;
    }
    *(short *)(puVar2 + 0x25) = (short)(0x1000000 / (iVar3 >> 0xc));
    FUN_8001d708/*0x8001d708*/(puVar2);
    FUN_800202f4/*0x800202f4*/(puVar2);
    *(int *)(_DAT_800659fc + 0x94) = *(int *)(_DAT_800659fc + 0x94) + 1;
  }
  *param_1 = *param_1 | 0x22;
  return 0;
}

