// addr: 0x80034920  name: FUN_80034920

uint FUN_80034920(uint *param_1,undefined4 param_2,uint param_3)

{
  ushort uVar1;
  undefined4 uVar2;
  uint uVar3;
  uint *puVar4;
  int iVar5;
  uint uVar6;
  
  switch(param_2) {
  case 0:
    FUN_8003c538(param_1,param_3);
    uVar6 = 0;
    break;
  case 1:
    *(undefined2 *)(param_1 + 3) = 10;
    *(undefined1 *)(param_1 + 2) = 4;
    *param_1 = *param_1 | 0x4000;
  default:
    uVar6 = 0;
    break;
  case 9:
    if ((param_3 & 0xfff) == 0x222) {
      if ((ushort)param_1[3] < 2) {
        return 0xffffffff;
      }
      uVar2 = FUN_8001d5a0(param_1);
      puVar4 = (uint *)FUN_800346cc(param_1,uVar2,0x24,0x1c,0x25);
      *(undefined1 *)(puVar4 + 2) = 1;
      *puVar4 = *puVar4 | 0x1000000;
      uVar1 = (short)param_1[3] - 2;
      *(ushort *)(param_1 + 3) = uVar1;
      uVar3 = (uint)uVar1;
    }
    else {
      if ((param_3 & 0xfff) != 0x224) {
        return 0;
      }
      if ((ushort)param_1[3] < 2) {
        return 0xffffffff;
      }
      uVar2 = FUN_8001d5a0(param_1);
      uVar6 = 5;
      if ((ushort)param_1[3] < 5) {
        uVar6 = (uint)(ushort)param_1[3];
      }
      puVar4 = (uint *)FUN_800346cc(param_1,uVar2,0x25,0x23,uVar6 * 0x4b);
      *(undefined1 *)(puVar4 + 2) = 2;
      *puVar4 = *puVar4 | 0x1000020;
      uVar6 = (ushort)param_1[3] - 5;
      uVar3 = 0;
      if (0 < (int)uVar6) {
        uVar3 = uVar6;
      }
      *(short *)(param_1 + 3) = (short)uVar3;
      uVar3 = uVar3 & 0xffff;
    }
    goto LAB_80034adc;
  case 0xb:
    if (*(short *)(param_3 + 0x11c) == 0) {
      uVar2 = 0x4b;
    }
    else {
      uVar2 = 0x96;
    }
    FUN_800346cc(param_1,param_3,0x10,8,uVar2);
    uVar1 = (short)param_1[3] - 1;
    *(ushort *)(param_1 + 3) = uVar1;
    uVar3 = (uint)uVar1;
LAB_80034adc:
    uVar6 = 0x78;
    if (uVar3 == 0) {
      FUN_8002cb7c(param_1);
      uVar6 = 0x78;
    }
    break;
  case 0xc:
    iVar5 = FUN_80016aac(param_3 + 0x48,*(int *)(param_3 + 0xe4) + 0x48);
    uVar6 = 0x1f3ffe < iVar5 - 0x1f4001U ^ 1;
    break;
  case 0xd:
    uVar6 = 4;
    break;
  case 0xe:
    uVar6 = 0x8014;
  }
  return uVar6;
}

