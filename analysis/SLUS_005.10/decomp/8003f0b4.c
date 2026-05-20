// addr: 0x8003f0b4  name: FUN_8003f0b4

void FUN_8003f0b4(int *param_1,uint param_2,uint *param_3)

{
  ushort uVar1;
  ushort uVar2;
  uint *puVar3;
  undefined4 uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  ushort *puVar8;
  
  puVar8 = (ushort *)(*param_1 + (param_2 & 0xffff) * 0x1c + 0x1c);
  uVar1 = *puVar8;
  uVar2 = uVar1 >> 0xc;
  if (uVar2 == 9) {
    uVar4 = FUN_8004410c();
    FUN_8004483c(uVar4,uRam000005f8,
                 *(undefined1 *)
                  (*(int *)((int)&PTR_s_6789__8005ed14 + (*puVar8 >> 6 & 0x3c)) + (*puVar8 & 0xff)),
                 param_3 + 5);
    goto LAB_8003f420;
  }
  if (uVar2 < 10) {
    if (uVar2 != 8) goto LAB_8003f420;
    if ((ushort)(uVar1 + 0x7c00) < 0xb) {
      FUN_8003fd24(param_3 + 5,*(undefined2 *)(&DAT_8005ece4 + (*puVar8 - 0x8400) * 2));
      goto LAB_8003f420;
    }
    if (uVar1 == 0x8500) {
      puVar3 = (uint *)FUN_80040c40(DAT_800737d8,7,param_3,&DAT_80065864,8);
    }
    else {
      if (uVar1 != 0x8501) {
        if (uVar1 == 0x8600) {
          FUN_8003fd24(param_3 + 5,0x24);
        }
        else if (uVar1 == 0x8601) {
          FUN_8003fd24(param_3 + 5,0x25);
        }
        else if (uVar1 == 0x8602) {
          FUN_8003fd24(param_3 + 5,0x26);
        }
        else if (uVar1 == 0x8700) {
          puVar3 = (uint *)FUN_8003fdcc(param_3 + 5,0x27,100);
          *puVar3 = *puVar3 | 0x10;
          FUN_8003fd24(param_3 + 5,0);
        }
        else {
          uVar1 = *puVar8;
          if (uVar1 == 0x8701) {
            FUN_8003fdcc(param_3 + 5,0x2e,100);
            FUN_8003fd24(param_3 + 5,0xc);
          }
          else if (uVar1 == 0x8702) {
            puVar3 = (uint *)FUN_8003fdcc(param_3 + 5,0x29,200);
            *puVar3 = *puVar3 | 0x10;
          }
          else if (uVar1 == 0x8703) {
            FUN_8003fdcc(param_3 + 5,0x30,200);
          }
          else if ((ushort)(uVar1 + 0x7800) < 8) {
            FUN_8003fea8(param_3 + 5,*(undefined4 *)(&DAT_8005ecf8 + (*puVar8 - 0x8800) * 4));
          }
        }
        goto LAB_8003f420;
      }
      puVar3 = (uint *)FUN_80040c40(DAT_800737d8,0xb,param_3,&DAT_80065864,8);
    }
  }
  else {
    if (uVar2 != 0xe) goto LAB_8003f420;
    *puVar8 = uVar1 & 0xfff;
    puVar3 = (uint *)FUN_8001ac44(param_1,param_2 & 0xffff,0x80,0);
    *puVar8 = *puVar8 | 0xe000;
    *(undefined1 *)(puVar3 + 1) = 1;
    puVar3[0x19] = (uint)&LAB_8003ed38;
    *puVar3 = *puVar3 | 0x10080;
    uVar5 = param_3[1];
    uVar6 = param_3[2];
    uVar7 = param_3[3];
    puVar3[4] = *param_3;
    puVar3[5] = uVar5;
    puVar3[6] = uVar6;
    puVar3[7] = uVar7;
    uVar5 = param_3[5];
    uVar6 = param_3[6];
    uVar7 = param_3[7];
    puVar3[8] = param_3[4];
    puVar3[9] = uVar5;
    puVar3[10] = uVar6;
    puVar3[0xb] = uVar7;
    *(undefined2 *)((int)puVar3 + 0x42) = 0xc;
    FUN_8001dc1c(puVar3);
  }
  FUN_800202f4(puVar3);
LAB_8003f420:
  if (puVar8[0xd] != 0xffff) {
    FUN_8003f4a0(param_1,puVar8[0xd],param_3,0);
  }
  return;
}

