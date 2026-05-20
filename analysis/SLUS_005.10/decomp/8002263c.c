// addr: 0x8002263c  name: FUN_8002263c

int FUN_8002263c(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  uint uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  int iVar7;
  int local_res0 [4];
  uint local_28 [2];
  
  uVar5 = 0;
  uVar6 = 0;
  iVar7 = 0;
  local_res0[0] = param_1;
joined_r0x8002266c:
  do {
    while( true ) {
      if (local_res0[0] == 0) {
        iVar1 = FUN_8001a640(uVar5,uVar6);
        if (param_2 != 0) {
          FUN_8001a91c(iVar1);
        }
        if (iVar7 != 0) {
          *(int *)(iVar1 + 8) = iVar7;
        }
        return iVar1;
      }
      iVar1 = FUN_80022524(local_28,local_res0);
      uVar4 = local_28[0] >> 0x18 | local_28[0] >> 8 & 0xff00 | (local_28[0] & 0xff00) << 8 |
              local_28[0] << 0x18;
      if (uVar4 != 0x42494e20) break;
      uVar2 = FUN_800116f4(iVar1);
      uVar5 = uVar2;
LAB_80022714:
      FUN_80015a20(uVar2,iVar1);
    }
    if (uVar4 < 0x42494e21) {
      if (uVar4 == 0x414e4d20) {
        uVar2 = FUN_800116f4(iVar1);
        uVar6 = uVar2;
        goto LAB_80022714;
      }
      goto joined_r0x8002266c;
    }
    if (uVar4 == 0x534e4420) {
      iVar3 = FUN_80015be4();
      iVar7 = FUN_800441f8();
      FUN_80015bf0(iVar3 + iVar1,0);
    }
  } while( true );
}

