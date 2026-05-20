// addr: 0x80044d9c  name: FUN_80044d9c

uint * FUN_80044d9c(uint *param_1,uint *param_2,int param_3)

{
  undefined1 *puVar1;
  uint *puVar2;
  uint *puVar3;
  undefined1 *puVar4;
  undefined1 *puVar5;
  uint uVar6;
  uint uVar7;
  undefined4 uVar8;
  uint uVar9;
  undefined4 uVar10;
  uint uVar11;
  undefined4 uVar12;
  uint uVar13;
  
  if ((int)param_2 < (int)param_1) {
    puVar4 = (undefined1 *)((int)param_1 + param_3);
    if (param_3 != 0) {
      puVar5 = (undefined1 *)((int)param_2 + param_3);
      while (((uint)puVar5 & 3) != 0) {
        puVar1 = puVar5 + -1;
        puVar5 = puVar5 + -1;
        puVar4[-1] = *puVar1;
        param_3 = param_3 + -1;
        puVar4 = puVar4 + -1;
        if (param_3 == 0) {
          return param_1;
        }
      }
      param_3 = param_3 + -0x10;
      if (((uint)puVar4 & 3) == 0) {
        uVar6 = 0;
        for (; -1 < param_3; param_3 = param_3 + -0x10) {
          uVar6 = *(uint *)(puVar5 + -4);
          uVar8 = *(undefined4 *)(puVar5 + -8);
          uVar10 = *(undefined4 *)(puVar5 + -0xc);
          uVar12 = *(undefined4 *)(puVar5 + -0x10);
          *(uint *)(puVar4 + -4) = uVar6;
          *(undefined4 *)(puVar4 + -8) = uVar8;
          *(undefined4 *)(puVar4 + -0xc) = uVar10;
          *(undefined4 *)(puVar4 + -0x10) = uVar12;
          puVar5 = puVar5 + -0x10;
          puVar4 = puVar4 + -0x10;
        }
        for (param_3 = param_3 + 0xc; -1 < param_3; param_3 = param_3 + -4) {
          uVar6 = *(uint *)(puVar5 + -4);
          puVar5 = puVar5 + -4;
          *(uint *)(puVar4 + -4) = uVar6;
          puVar4 = puVar4 + -4;
        }
        param_3 = param_3 + 4;
        if (0 < param_3) {
          uVar7 = (int)puVar5 - param_3 & 3;
          uVar9 = (int)puVar4 - param_3 & 3;
          puVar3 = (uint *)(((int)puVar4 - param_3) - uVar9);
          *puVar3 = *puVar3 & 0xffffffffU >> (4 - uVar9) * 8 |
                    (uVar6 & -1 << (4 - uVar7) * 8 |
                    *(uint *)(((int)puVar5 - param_3) - uVar7) >> uVar7 * 8) << uVar9 * 8;
          return param_1;
        }
      }
      else {
        for (; -1 < param_3; param_3 = param_3 + -0x10) {
          uVar7 = *(uint *)(puVar5 + -4);
          uVar9 = *(uint *)(puVar5 + -8);
          uVar11 = *(uint *)(puVar5 + -0xc);
          uVar13 = *(uint *)(puVar5 + -0x10);
          puVar1 = puVar4 + -4;
          uVar6 = (uint)puVar1 & 3;
          *(uint *)(puVar1 + -uVar6) =
               *(uint *)(puVar1 + -uVar6) & 0xffffffffU >> (4 - uVar6) * 8 | uVar7 << uVar6 * 8;
          puVar1 = puVar4 + -1;
          uVar6 = (uint)puVar1 & 3;
          *(uint *)(puVar1 + -uVar6) =
               *(uint *)(puVar1 + -uVar6) & -1 << (uVar6 + 1) * 8 | uVar7 >> (3 - uVar6) * 8;
          puVar1 = puVar4 + -8;
          uVar6 = (uint)puVar1 & 3;
          *(uint *)(puVar1 + -uVar6) =
               *(uint *)(puVar1 + -uVar6) & 0xffffffffU >> (4 - uVar6) * 8 | uVar9 << uVar6 * 8;
          puVar1 = puVar4 + -5;
          uVar6 = (uint)puVar1 & 3;
          *(uint *)(puVar1 + -uVar6) =
               *(uint *)(puVar1 + -uVar6) & -1 << (uVar6 + 1) * 8 | uVar9 >> (3 - uVar6) * 8;
          puVar1 = puVar4 + -0xc;
          uVar6 = (uint)puVar1 & 3;
          *(uint *)(puVar1 + -uVar6) =
               *(uint *)(puVar1 + -uVar6) & 0xffffffffU >> (4 - uVar6) * 8 | uVar11 << uVar6 * 8;
          puVar1 = puVar4 + -9;
          uVar6 = (uint)puVar1 & 3;
          *(uint *)(puVar1 + -uVar6) =
               *(uint *)(puVar1 + -uVar6) & -1 << (uVar6 + 1) * 8 | uVar11 >> (3 - uVar6) * 8;
          puVar1 = puVar4 + -0x10;
          uVar6 = (uint)puVar1 & 3;
          *(uint *)(puVar1 + -uVar6) =
               *(uint *)(puVar1 + -uVar6) & 0xffffffffU >> (4 - uVar6) * 8 | uVar13 << uVar6 * 8;
          puVar1 = puVar4 + -0xd;
          uVar6 = (uint)puVar1 & 3;
          *(uint *)(puVar1 + -uVar6) =
               *(uint *)(puVar1 + -uVar6) & -1 << (uVar6 + 1) * 8 | uVar13 >> (3 - uVar6) * 8;
          puVar5 = puVar5 + -0x10;
          puVar4 = puVar4 + -0x10;
        }
        for (param_3 = param_3 + 0xc; -1 < param_3; param_3 = param_3 + -4) {
          uVar7 = *(uint *)(puVar5 + -4);
          puVar5 = puVar5 + -4;
          puVar1 = puVar4 + -4;
          uVar6 = (uint)puVar1 & 3;
          *(uint *)(puVar1 + -uVar6) =
               *(uint *)(puVar1 + -uVar6) & 0xffffffffU >> (4 - uVar6) * 8 | uVar7 << uVar6 * 8;
          puVar1 = puVar4 + -1;
          uVar6 = (uint)puVar1 & 3;
          *(uint *)(puVar1 + -uVar6) =
               *(uint *)(puVar1 + -uVar6) & -1 << (uVar6 + 1) * 8 | uVar7 >> (3 - uVar6) * 8;
          puVar4 = puVar4 + -4;
        }
        for (param_3 = param_3 + 4; param_3 != 0; param_3 = param_3 + -1) {
          puVar1 = puVar5 + -1;
          puVar5 = puVar5 + -1;
          puVar4[-1] = *puVar1;
          puVar4 = puVar4 + -1;
        }
      }
    }
    return param_1;
  }
  puVar3 = param_1;
  if (param_3 != 0) {
    while (((uint)param_2 & 3) != 0) {
      uVar6 = *param_2;
      param_2 = (uint *)((int)param_2 + 1);
      *(char *)param_1 = (char)uVar6;
      param_3 = param_3 + -1;
      param_1 = (uint *)((int)param_1 + 1);
      if (param_3 == 0) {
        return puVar3;
      }
    }
    param_3 = param_3 + -0x10;
    if (((uint)param_1 & 3) == 0) {
      uVar6 = 0;
      for (; -1 < param_3; param_3 = param_3 + -0x10) {
        uVar6 = *param_2;
        uVar7 = param_2[1];
        uVar9 = param_2[2];
        uVar11 = param_2[3];
        *param_1 = uVar6;
        param_1[1] = uVar7;
        param_1[2] = uVar9;
        param_1[3] = uVar11;
        param_2 = param_2 + 4;
        param_1 = param_1 + 4;
      }
      for (param_3 = param_3 + 0xc; -1 < param_3; param_3 = param_3 + -4) {
        uVar6 = *param_2;
        param_2 = param_2 + 1;
        *param_1 = uVar6;
        param_1 = param_1 + 1;
      }
      param_3 = param_3 + 3;
      if (-1 < param_3) {
        uVar7 = (uint)((int)param_2 + param_3) & 3;
        uVar9 = (uint)((int)param_1 + param_3) & 3;
        puVar2 = (uint *)((undefined1 *)((int)param_1 + param_3) + -uVar9);
        *puVar2 = *puVar2 & -1 << (uVar9 + 1) * 8 |
                  (*(int *)((undefined1 *)((int)param_2 + param_3) + -uVar7) << (3 - uVar7) * 8 |
                  uVar6 & 0xffffffffU >> (uVar7 + 1) * 8) >> (3 - uVar9) * 8;
        return puVar3;
      }
    }
    else {
      for (; -1 < param_3; param_3 = param_3 + -0x10) {
        uVar7 = *param_2;
        uVar9 = param_2[1];
        uVar11 = param_2[2];
        uVar13 = param_2[3];
        uVar6 = (uint)param_1 & 3;
        *(uint *)((int)param_1 - uVar6) =
             *(uint *)((int)param_1 - uVar6) & 0xffffffffU >> (4 - uVar6) * 8 | uVar7 << uVar6 * 8;
        uVar6 = (int)param_1 + 3U & 3;
        puVar2 = (uint *)((undefined1 *)((int)param_1 + 3U) + -uVar6);
        *puVar2 = *puVar2 & -1 << (uVar6 + 1) * 8 | uVar7 >> (3 - uVar6) * 8;
        uVar6 = (uint)(param_1 + 1) & 3;
        puVar2 = (uint *)((int)(param_1 + 1) - uVar6);
        *puVar2 = *puVar2 & 0xffffffffU >> (4 - uVar6) * 8 | uVar9 << uVar6 * 8;
        uVar6 = (int)param_1 + 7U & 3;
        puVar2 = (uint *)((undefined1 *)((int)param_1 + 7U) + -uVar6);
        *puVar2 = *puVar2 & -1 << (uVar6 + 1) * 8 | uVar9 >> (3 - uVar6) * 8;
        uVar6 = (uint)(param_1 + 2) & 3;
        puVar2 = (uint *)((int)(param_1 + 2) - uVar6);
        *puVar2 = *puVar2 & 0xffffffffU >> (4 - uVar6) * 8 | uVar11 << uVar6 * 8;
        uVar6 = (int)param_1 + 0xbU & 3;
        puVar2 = (uint *)((undefined1 *)((int)param_1 + 0xbU) + -uVar6);
        *puVar2 = *puVar2 & -1 << (uVar6 + 1) * 8 | uVar11 >> (3 - uVar6) * 8;
        uVar6 = (uint)(param_1 + 3) & 3;
        puVar2 = (uint *)((int)(param_1 + 3) - uVar6);
        *puVar2 = *puVar2 & 0xffffffffU >> (4 - uVar6) * 8 | uVar13 << uVar6 * 8;
        uVar6 = (int)param_1 + 0xfU & 3;
        puVar2 = (uint *)((undefined1 *)((int)param_1 + 0xfU) + -uVar6);
        *puVar2 = *puVar2 & -1 << (uVar6 + 1) * 8 | uVar13 >> (3 - uVar6) * 8;
        param_2 = param_2 + 4;
        param_1 = param_1 + 4;
      }
      for (param_3 = param_3 + 0xc; -1 < param_3; param_3 = param_3 + -4) {
        uVar7 = *param_2;
        param_2 = param_2 + 1;
        uVar6 = (uint)param_1 & 3;
        *(uint *)((int)param_1 - uVar6) =
             *(uint *)((int)param_1 - uVar6) & 0xffffffffU >> (4 - uVar6) * 8 | uVar7 << uVar6 * 8;
        uVar6 = (int)param_1 + 3U & 3;
        puVar2 = (uint *)((undefined1 *)((int)param_1 + 3U) + -uVar6);
        *puVar2 = *puVar2 & -1 << (uVar6 + 1) * 8 | uVar7 >> (3 - uVar6) * 8;
        param_1 = param_1 + 1;
      }
      for (param_3 = param_3 + 4; param_3 != 0; param_3 = param_3 + -1) {
        uVar6 = *param_2;
        param_2 = (uint *)((int)param_2 + 1);
        *(char *)param_1 = (char)uVar6;
        param_1 = (uint *)((int)param_1 + 1);
      }
    }
  }
  return puVar3;
}

