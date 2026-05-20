// addr: 0x801072ec  name: FUN_801072ec

void FUN_801072ec(int param_1,short param_2)

{
  ushort uVar1;
  int *piVar2;
  uint uVar3;
  undefined **ppuVar4;
  undefined2 local_30;
  short sStack_2e;
  undefined4 local_2c;
  undefined4 local_28;
  int local_24;
  undefined4 local_20;
  undefined4 local_1c;
  
  piVar2 = (int *)FUN_80019034/*0x80019034*/(DAT_80113388 + *(int *)(DAT_80113388 + 8),1);
  uVar1 = *(byte *)(*piVar2 + 6) + 2;
  local_24 = (uint)uVar1 << 0x10;
  local_30 = 0x20;
  sStack_2e = 0x80;
  local_2c = local_24;
  local_1c = CONCAT22(uVar1,0x10);
  local_20 = CONCAT22(uVar1 * param_2 + 0x80,0x10);
  local_28 = local_20;
  local_24 = local_1c;
  uVar3 = 0;
  FUN_80019e20/*0x80019e20*/();
  ppuVar4 = &PTR_s_Game_Status_80111ee4;
  do {
    piVar2[1] = piVar2[1] & 0xff000000U | 0x576044;
    FUN_80019a58/*0x80019a58*/(piVar2,*ppuVar4,&local_30,8);
    uVar3 = uVar3 + 1;
    sStack_2e = sStack_2e + local_2c._2_2_;
    ppuVar4 = ppuVar4 + 1;
  } while (uVar3 < 8);
  if (param_1 == 0) {
    *(undefined1 *)(piVar2 + 1) = 0x80;
    *(undefined1 *)((int)piVar2 + 5) = 0x80;
    *(undefined1 *)((int)piVar2 + 6) = 0x80;
  }
  FUN_80019a58/*0x80019a58*/(piVar2,&DAT_80100938,&local_28,8);
  FUN_800190a8/*0x800190a8*/(piVar2);
  return;
}

