// addr: 0x80053a74  name: SetRCnt

long SetRCnt(ulong param_1,ushort param_2,long param_3)

{
  long lVar1;
  ushort uVar2;
  uint uVar3;
  
  uVar3 = param_1 & 0xffff;
  uVar2 = 0x48;
  if (2 < uVar3) {
    lVar1 = COUNTER_OBJ_94();
    return lVar1;
  }
  *(undefined2 *)(&TMR_DOTCLOCK_MODE + uVar3 * 4) = 0;
  *(ushort *)(&TMR_DOTCLOCK_MAX + uVar3 * 4) = param_2;
  if (uVar3 < 2) {
    uVar2 = 0x48;
    if ((param_3 & 0x10U) != 0) {
      uVar2 = 0x49;
    }
    if ((param_3 & 1U) == 0) {
      lVar1 = COUNTER_OBJ_74();
      return lVar1;
    }
  }
  else if ((uVar3 == 2) && ((param_3 & 1U) == 0)) {
    uVar2 = 0x248;
  }
  if ((param_3 & 0x1000U) != 0) {
    uVar2 = uVar2 | 0x10;
  }
  *(ushort *)(&TMR_DOTCLOCK_MODE + uVar3 * 4) = uVar2;
  return 1;
}

