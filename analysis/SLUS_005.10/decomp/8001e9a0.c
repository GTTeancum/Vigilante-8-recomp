// addr: 0x8001e9a0  name: FUN_8001e9a0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_8001e9a0(int param_1,int param_2,undefined4 param_3,undefined4 param_4)

{
  short sVar1;
  int iVar2;
  int iVar3;
  short *psVar4;
  int iVar5;
  short *psVar6;
  
  psVar6 = *(short **)(param_1 + 0x5c);
  if (psVar6 != (short *)0x0) {
    if (*(int *)(param_2 + 0x5c) == 0) {
      return 0;
    }
    sVar1 = *psVar6;
    while (sVar1 != 0) {
      sVar1 = *psVar6;
      psVar4 = *(short **)(param_2 + 0x5c);
      if (sVar1 == 1) {
        if (*psVar4 != 0) {
          do {
            sVar1 = *psVar4;
            if (sVar1 == 1) {
              iVar3 = FUN_8001e1c0(psVar6 + 2,param_3,psVar4 + 2,param_4);
              if ((iVar3 != 0) &&
                 (iVar3 = FUN_8001e1c0(psVar4 + 2,param_4,psVar6 + 2,param_3), iVar3 != 0)) {
                DAT_1f800004 = psVar6;
                DAT_1f800008 = psVar4;
                _DAT_1f80000c = param_1;
                DAT_1f800010 = param_2;
                return 0x1f800000;
              }
              psVar4 = psVar4 + 0xe;
LAB_8001eb04:
              sVar1 = *psVar4;
            }
            else if (sVar1 == 2) {
              iVar3 = 0;
              if (psVar4[1] == 0) {
                DAT_1f800004 = psVar6;
                DAT_1f800008 = psVar4;
                _DAT_1f80000c = param_1;
                DAT_1f800010 = param_2;
                return 0x1f800000;
              }
              iVar5 = 4;
              while( true ) {
                iVar2 = FUN_8001e408(psVar6 + 2,param_3,(int)psVar4 + iVar5,param_4);
                iVar3 = iVar3 + 1;
                if (iVar2 == 0) break;
                iVar5 = iVar5 + 0xc;
                if ((int)(uint)(ushort)psVar4[1] <= iVar3) {
                  DAT_1f800004 = psVar6;
                  DAT_1f800008 = psVar4;
                  _DAT_1f80000c = param_1;
                  DAT_1f800010 = param_2;
                  return 0x1f800000;
                }
              }
              psVar4 = psVar4 + (uint)(ushort)psVar4[1] * 6 + 2;
              goto LAB_8001eb04;
            }
          } while (sVar1 != 0);
        }
        psVar6 = psVar6 + 0xe;
LAB_8001ebd8:
        sVar1 = *psVar6;
      }
      else if (sVar1 == 2) {
LAB_8001ebac:
        do {
          sVar1 = *psVar4;
          while( true ) {
            if (sVar1 == 0) {
              psVar6 = psVar6 + (uint)(ushort)psVar6[1] * 6 + 2;
              goto LAB_8001ebd8;
            }
            sVar1 = *psVar4;
            if (sVar1 == 1) break;
            if (sVar1 == 2) goto LAB_8001eb90;
          }
          iVar3 = 0;
          if (psVar6[1] == 0) {
            DAT_1f800004 = psVar6;
            DAT_1f800008 = psVar4;
            _DAT_1f80000c = param_1;
            DAT_1f800010 = param_2;
            return 0x1f800000;
          }
          iVar5 = 4;
          while( true ) {
            iVar2 = FUN_8001e408(psVar4 + 2,param_4,(int)psVar6 + iVar5,param_3);
            iVar3 = iVar3 + 1;
            if (iVar2 == 0) break;
            iVar5 = iVar5 + 0xc;
            if ((int)(uint)(ushort)psVar6[1] <= iVar3) {
              DAT_1f800004 = psVar6;
              DAT_1f800008 = psVar4;
              _DAT_1f80000c = param_1;
              DAT_1f800010 = param_2;
              return 0x1f800000;
            }
          }
          psVar4 = psVar4 + 0xe;
        } while( true );
      }
    }
  }
  return 0;
LAB_8001eb90:
  psVar4 = psVar4 + (uint)(ushort)psVar4[1] * 6 + 2;
  goto LAB_8001ebac;
}

