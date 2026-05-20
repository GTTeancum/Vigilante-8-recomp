// addr: 0x801002ac  name: FUN_801002ac

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_801002ac(undefined4 param_1,int param_2)

{
  undefined2 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int iVar5;
  undefined2 *puVar6;
  int iVar7;
  undefined4 local_res0 [4];
  
  local_res0[0] = param_1;
  uVar2 = func_0x800224b4(local_res0);
  uVar3 = func_0x800224b4(local_res0);
  uVar4 = func_0x800224b4(local_res0);
  func_0x800224b4(local_res0);
  func_0x800224b4(local_res0);
  iVar5 = (param_2 + -10) / 2;
  iVar7 = 0;
  if (0 < iVar5) {
    puVar6 = (undefined2 *)&DAT_800658e8;
    do {
      uVar1 = func_0x800224b4(local_res0);
      *puVar6 = uVar1;
      iVar7 = iVar7 + 1;
      puVar6 = puVar6 + 1;
    } while (iVar7 < iVar5);
  }
  _DAT_800658ee = (short)(((int)_DAT_800658ee << 1) >> ((int)DAT_8006531a & 0x1fU));
  Terrain_InitFlatWorld/*0x800251fc*/(uVar2);
  func_0x80041d40(uVar3,uVar4);
  return;
}

