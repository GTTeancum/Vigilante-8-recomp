// addr: 0x80101d00  name: FUN_80101d00

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_80101d00(uint *param_1,uint param_2,undefined4 *param_3)

{
  bool bVar1;
  int *piVar2;
  uint uVar3;
  char cVar4;
  int iVar5;
  uint uVar6;
  undefined4 uVar7;
  uint *puVar8;
  uint *puVar9;
  int *piVar10;
  
  if (param_2 != 2) {
    if (((param_2 < 3) && (param_2 == 1)) || ((param_2 != 3 && (param_2 == 6)))) goto LAB_80101fa4;
    puVar9 = (uint *)*param_3;
    if ((char)puVar9[1] != '\x02') {
      return 0;
    }
    puVar8 = param_1 + 0x20;
    if ((*puVar9 & 0x4000) != 0) {
      iVar5 = FUN_8001fe50/*0x8001fe50*/(puVar8,puVar9);
      *(uint *)(iVar5 + 0xc) = _DAT_80065310 + 300 | 1;
      if ((*param_1 & 1) == 0) {
        FUN_80020890/*0x80020890*/(param_1,0x3c);
      }
      *puVar9 = *puVar9 & 0xffffbfff;
      FUN_8002002c/*0x8002002c*/(0x80065a18,9,puVar9,0x58a,4,0x3c);
      puVar8 = (uint *)0x1;
    }
    iVar5 = FUN_8001ff0c/*0x8001ff0c*/(puVar8,puVar9);
    *(uint *)(iVar5 + 0xc) = *(uint *)(iVar5 + 0xc) | 1;
  }
  bVar1 = false;
  piVar10 = (int *)param_1[0x20];
  uVar3 = _DAT_80065310;
  for (piVar2 = (int *)*(int *)param_1[0x20]; _DAT_80065310 = uVar3, piVar2 != (int *)0x0;
      piVar2 = (int *)*piVar2) {
    if (((piVar10[3] & 1U) != 0) && (*(short *)(piVar10[2] + 0xc) != 0)) {
      uVar6 = piVar10[3] & 0xfffffffe;
      piVar10[3] = uVar6;
      if (uVar6 < uVar3) {
        FUN_8002c6fc/*0x8002c6fc*/(piVar10[2],0xffffffec,&DAT_80100110,1);
        bVar1 = true;
        if (*(char *)((int)param_1 + 5) == '\0') {
          cVar4 = FUN_8004410c/*0x8004410c*/();
          *(char *)((int)param_1 + 5) = cVar4;
          FUN_800443c8/*0x800443c8*/((int)cVar4,*(undefined4 *)(param_1[0x16] + 8),2,0);
        }
      }
      FUN_8002002c/*0x8002002c*/(0x80065a18,9,piVar10[2]);
      piVar10 = piVar2;
    }
    *(uint *)piVar10[2] = *(uint *)piVar10[2] | 0x4000;
    func_0x80022c78(piVar10);
    piVar10 = piVar2;
    uVar3 = _DAT_80065310;
  }
  if (bVar1) {
LAB_80101f58:
    if (*(char *)((int)param_1 + 5) != '\0') {
      uVar7 = FUN_800446dc/*0x800446dc*/(param_1 + 0x12);
      FUN_80044574/*0x80044574*/((int)*(char *)((int)param_1 + 5),uVar7);
    }
  }
  else if (*(char *)((int)param_1 + 5) != '\0') {
    FUN_800441c8/*0x800441c8*/();
    *(undefined1 *)((int)param_1 + 5) = 0;
    goto LAB_80101f58;
  }
  if ((uint *)param_1[0x22] == param_1 + 0x20) {
    return 0;
  }
  FUN_80020890/*0x80020890*/(param_1,0x3c);
LAB_80101fa4:
  *(undefined1 *)(param_1 + 1) = 3;
  param_1[0x20] = (uint)(param_1 + 0x21);
  param_1[0x21] = 0;
  param_1[0x22] = (uint)(param_1 + 0x20);
  return 0;
}

