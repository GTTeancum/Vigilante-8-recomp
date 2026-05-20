// addr: 0x80015c68  name: FUN_80015c68

int FUN_80015c68(int param_1,undefined4 param_2,int param_3)

{
  uint uVar1;
  byte *pbVar2;
  int iVar3;
  uint uVar4;
  byte *pbVar5;
  int iVar6;
  uint uVar7;
  byte bVar8;
  
  FUN_8001570c(param_2);
  *(undefined4 *)(param_1 + 0x10) = 0;
  *(undefined4 *)(param_1 + 8) = 0;
  do {
    if (param_3 == 0) {
      FUN_80015798();
      iVar6 = FUN_80045134(param_1,*(int *)(param_1 + 0x10) * 0x14 + 0x14);
      for (iVar3 = *(int *)(iVar6 + 8); iVar3 != 0; iVar3 = *(int *)(iVar3 + 0xc)) {
        FUN_80015c68(iVar3,*(undefined4 *)(iVar3 + 0x20),*(undefined4 *)(iVar3 + 0x24));
      }
      return iVar6;
    }
    pbVar2 = (byte *)FUN_800156d4();
    bVar8 = *pbVar2;
    while (bVar8 != 0) {
      uVar7 = 0;
      if ((pbVar2[0x19] & 2) == 0) {
        iVar3 = param_1 + *(int *)(param_1 + 0x10) * 0x14 + 0x14;
        *(int *)(param_1 + 0x10) = *(int *)(param_1 + 0x10) + 1;
        pbVar5 = pbVar2;
        do {
          if (pbVar5[0x21] == 0x3b) break;
          *(byte *)(iVar3 + uVar7) = pbVar5[0x21];
          uVar7 = uVar7 + 1;
          pbVar5 = pbVar2 + uVar7;
        } while (uVar7 < 0xc);
        for (; uVar7 < 0xc; uVar7 = uVar7 + 1) {
                    /* Possible PsyQ macro: setPolyF3() */
          *(undefined1 *)(iVar3 + uVar7) = 0x20;
        }
        uVar1 = (uint)(pbVar2 + 5) & 3;
        uVar4 = (uint)(pbVar2 + 2) & 3;
        uVar4 = (*(int *)(pbVar2 + 5 + -uVar1) << (3 - uVar1) * 8 |
                iVar3 + uVar7 & 0xffffffffU >> (uVar1 + 1) * 8) & -1 << (4 - uVar4) * 8 |
                *(uint *)(pbVar2 + 2 + -uVar4) >> uVar4 * 8;
        *(uint *)(iVar3 + 0xc) = uVar4;
        uVar7 = (uint)(pbVar2 + 0xd) & 3;
        uVar1 = (uint)(pbVar2 + 10) & 3;
        *(uint *)(iVar3 + 0x10) =
             (*(int *)(pbVar2 + 0xd + -uVar7) << (3 - uVar7) * 8 |
             uVar4 & 0xffffffffU >> (uVar7 + 1) * 8) & -1 << (4 - uVar1) * 8 |
             *(uint *)(pbVar2 + 10 + -uVar1) >> uVar1 * 8;
      }
      else if (1 < pbVar2[0x21]) {
        iVar3 = FUN_800116f4(0x514);
        FUN_80044efc(iVar3,0x20,8);
        bVar8 = 8;
        if (pbVar2[0x20] < 8) {
          bVar8 = pbVar2[0x20];
        }
        uVar4 = FUN_80044c44(iVar3,pbVar2 + 0x21,bVar8);
        uVar7 = (uint)(pbVar2 + 5) & 3;
        uVar1 = (uint)(pbVar2 + 2) & 3;
        uVar4 = (*(int *)(pbVar2 + 5 + -uVar7) << (3 - uVar7) * 8 |
                uVar4 & 0xffffffffU >> (uVar7 + 1) * 8) & -1 << (4 - uVar1) * 8 |
                *(uint *)(pbVar2 + 2 + -uVar1) >> uVar1 * 8;
        *(uint *)(iVar3 + 0x20) = uVar4;
        uVar7 = (uint)(pbVar2 + 0xd) & 3;
        uVar1 = (uint)(pbVar2 + 10) & 3;
        *(uint *)(iVar3 + 0x24) =
             (*(int *)(pbVar2 + 0xd + -uVar7) << (3 - uVar7) * 8 |
             uVar4 & 0xffffffffU >> (uVar7 + 1) * 8) & -1 << (4 - uVar1) * 8 |
             *(uint *)(pbVar2 + 10 + -uVar1) >> uVar1 * 8;
        *(undefined4 *)(iVar3 + 0xc) = *(undefined4 *)(param_1 + 8);
        *(int *)(param_1 + 8) = iVar3;
      }
      pbVar2 = pbVar2 + *pbVar2;
      bVar8 = *pbVar2;
    }
    param_3 = param_3 + -0x800;
  } while( true );
}

