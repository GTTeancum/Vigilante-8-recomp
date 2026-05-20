// addr: 0x801004cc  name: FUN_801004cc

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_801004cc(uint *param_1,int param_2,int *param_3)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  
  if ((param_2 == 1) || (param_2 != 3)) {
    iVar1 = FUN_8001ac44/*0x8001ac44*/(_DAT_800737d8,0x26,0x80,8);
    *(undefined1 *)(iVar1 + 4) = 3;
    iVar2 = FUN_80017160/*0x80017160*/();
    *(short *)(iVar1 + 0x46) = (short)(iVar2 * **(int **)(_DAT_800737d8 + 4) >> 0xf);
    uVar3 = FUN_8001b038/*0x8001b038*/(param_1,0x8000);
    FUN_8001b2fc/*0x8001b2fc*/(param_1,uVar3,iVar1);
    *param_1 = *param_1 | 4;
  }
  if (*(char *)(*param_3 + 4) == '\x02') {
    FUN_8002c3ac/*0x8002c3ac*/();
  }
  func_0x800223dc(param_1,param_2,param_3);
  return;
}

