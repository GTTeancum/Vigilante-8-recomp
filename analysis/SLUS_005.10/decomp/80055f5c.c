// addr: 0x80055f5c  name: PADPORTD_OBJ_388

/* WARNING: Removing unreachable block (ram,0x80056084) */
/* WARNING: Removing unreachable block (ram,0x80056094) */
/* WARNING: Removing unreachable block (ram,0x800560a4) */
/* WARNING: Removing unreachable block (ram,0x800560a8) */

void PADPORTD_OBJ_388(int param_1)

{
  int iVar1;
  byte *pbVar2;
  byte *pbVar3;
  bool bVar4;
  byte bVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  
  FUN_80044f64(param_1 + 0x57,6);
  if ((*(short *)(param_1 + 0xe6) == 0) || (*(int *)(param_1 + 0x28) == 0)) {
    if (((*(byte *)(param_1 + 0xe8) - 4 < 2) || (*(byte *)(param_1 + 0xe8) == 7)) &&
       ((*(short *)(param_1 + 0xe6) == 0 && (1 < *(byte *)(param_1 + 0x34))))) {
      if ((((**(byte **)(param_1 + 0x28) & 0xc0) == 0x40) &&
          (((*(byte **)(param_1 + 0x28))[1] & 1) != 0)) && (DAT_800652a4 + 10 < 0x3d)) {
        *(undefined1 *)(param_1 + 0x58) = 1;
        *(undefined1 *)(param_1 + 0x57) = 1;
        DAT_800652a4 = DAT_800652a4 + 10;
        PADPORTD_OBJ_5F4();
        return;
      }
    }
    else {
      if (*(char *)(param_1 + 0xe8) == '\x03') {
        *(undefined1 *)(param_1 + 0x57) = 1;
        PADPORTD_OBJ_5F4();
        return;
      }
      if (*(short *)(param_1 + 0xe6) == 0) {
        iVar8 = 5;
        param_1 = param_1 + 5;
        do {
          *(undefined1 *)(param_1 + 0x57) = 1;
          iVar8 = iVar8 + -1;
          param_1 = param_1 + -1;
        } while (-1 < iVar8);
      }
    }
  }
  else {
    uVar7 = 6;
    if (*(byte *)(param_1 + 0x34) < 7) {
      uVar7 = (uint)*(byte *)(param_1 + 0x34);
    }
    uVar6 = 0;
    if (*(char *)(param_1 + 0xe9) != '\0') {
      iVar8 = 0;
      do {
        bVar4 = false;
        bVar5 = 1;
        if (*(char *)(iVar8 + *(int *)(param_1 + 4) + 2) != '\0') {
          bVar5 = 0xff;
        }
        pbVar3 = (byte *)(param_1 + 0x5d);
        pbVar2 = *(byte **)(param_1 + 0x28);
        iVar1 = 0;
        if (uVar7 != 0) {
          do {
            if ((*pbVar3 == uVar6) && ((*pbVar2 & bVar5) != 0)) {
              bVar4 = true;
              break;
            }
            pbVar3 = pbVar3 + 1;
            iVar1 = iVar1 + 1;
            pbVar2 = pbVar2 + 1;
          } while (iVar1 < (int)uVar7);
        }
        if ((bVar4) &&
           (iVar1 = DAT_800652a4 + (uint)*(byte *)(iVar8 + *(int *)(param_1 + 4) + 3), iVar1 < 0x3d)
           ) {
          DAT_800652a4 = iVar1;
          PADPORTD_OBJ_4A8();
          return;
        }
        uVar6 = uVar6 + 1;
        iVar8 = iVar8 + 5;
        if ((int)(uint)*(byte *)(param_1 + 0xe9) <= (int)uVar6) {
          PADPORTD_OBJ_5F4();
          return;
        }
      } while( true );
    }
  }
  return;
}

