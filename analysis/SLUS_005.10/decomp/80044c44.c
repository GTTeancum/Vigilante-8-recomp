// addr: 0x80044c44  name: FUN_80044c44

uint * FUN_80044c44(uint *param_1,uint *param_2,int param_3)

{
  uint *puVar1;
  uint *puVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  
  puVar2 = param_1;
  if (param_3 != 0) {
    while (((uint)param_2 & 3) != 0) {
      uVar3 = *param_2;
      param_2 = (uint *)((int)param_2 + 1);
      *(char *)param_1 = (char)uVar3;
      param_3 = param_3 + -1;
      param_1 = (uint *)((int)param_1 + 1);
      if (param_3 == 0) {
        return puVar2;
      }
    }
    param_3 = param_3 + -0x10;
    if (((uint)param_1 & 3) == 0) {
      uVar3 = 0;
      for (; -1 < param_3; param_3 = param_3 + -0x10) {
        uVar3 = *param_2;
        uVar4 = param_2[1];
        uVar5 = param_2[2];
        uVar6 = param_2[3];
        *param_1 = uVar3;
        param_1[1] = uVar4;
        param_1[2] = uVar5;
        param_1[3] = uVar6;
        param_2 = param_2 + 4;
        param_1 = param_1 + 4;
      }
      for (param_3 = param_3 + 0xc; -1 < param_3; param_3 = param_3 + -4) {
        uVar3 = *param_2;
        param_2 = param_2 + 1;
        *param_1 = uVar3;
        param_1 = param_1 + 1;
      }
      param_3 = param_3 + 3;
      if (-1 < param_3) {
        uVar4 = (uint)((int)param_2 + param_3) & 3;
        uVar5 = (uint)((int)param_1 + param_3) & 3;
        puVar1 = (uint *)((undefined1 *)((int)param_1 + param_3) + -uVar5);
        *puVar1 = *puVar1 & -1 << (uVar5 + 1) * 8 |
                  (*(int *)((undefined1 *)((int)param_2 + param_3) + -uVar4) << (3 - uVar4) * 8 |
                  uVar3 & 0xffffffffU >> (uVar4 + 1) * 8) >> (3 - uVar5) * 8;
        return puVar2;
      }
    }
    else {
      for (; -1 < param_3; param_3 = param_3 + -0x10) {
        uVar4 = *param_2;
        uVar5 = param_2[1];
        uVar6 = param_2[2];
        uVar7 = param_2[3];
        uVar3 = (uint)param_1 & 3;
        *(uint *)((int)param_1 - uVar3) =
             *(uint *)((int)param_1 - uVar3) & 0xffffffffU >> (4 - uVar3) * 8 | uVar4 << uVar3 * 8;
        uVar3 = (int)param_1 + 3U & 3;
        puVar1 = (uint *)((undefined1 *)((int)param_1 + 3U) + -uVar3);
        *puVar1 = *puVar1 & -1 << (uVar3 + 1) * 8 | uVar4 >> (3 - uVar3) * 8;
        uVar3 = (uint)(param_1 + 1) & 3;
        puVar1 = (uint *)((int)(param_1 + 1) - uVar3);
        *puVar1 = *puVar1 & 0xffffffffU >> (4 - uVar3) * 8 | uVar5 << uVar3 * 8;
        uVar3 = (int)param_1 + 7U & 3;
        puVar1 = (uint *)((undefined1 *)((int)param_1 + 7U) + -uVar3);
        *puVar1 = *puVar1 & -1 << (uVar3 + 1) * 8 | uVar5 >> (3 - uVar3) * 8;
        uVar3 = (uint)(param_1 + 2) & 3;
        puVar1 = (uint *)((int)(param_1 + 2) - uVar3);
        *puVar1 = *puVar1 & 0xffffffffU >> (4 - uVar3) * 8 | uVar6 << uVar3 * 8;
        uVar3 = (int)param_1 + 0xbU & 3;
        puVar1 = (uint *)((undefined1 *)((int)param_1 + 0xbU) + -uVar3);
        *puVar1 = *puVar1 & -1 << (uVar3 + 1) * 8 | uVar6 >> (3 - uVar3) * 8;
        uVar3 = (uint)(param_1 + 3) & 3;
        puVar1 = (uint *)((int)(param_1 + 3) - uVar3);
        *puVar1 = *puVar1 & 0xffffffffU >> (4 - uVar3) * 8 | uVar7 << uVar3 * 8;
        uVar3 = (int)param_1 + 0xfU & 3;
        puVar1 = (uint *)((undefined1 *)((int)param_1 + 0xfU) + -uVar3);
        *puVar1 = *puVar1 & -1 << (uVar3 + 1) * 8 | uVar7 >> (3 - uVar3) * 8;
        param_2 = param_2 + 4;
        param_1 = param_1 + 4;
      }
      for (param_3 = param_3 + 0xc; -1 < param_3; param_3 = param_3 + -4) {
        uVar4 = *param_2;
        param_2 = param_2 + 1;
        uVar3 = (uint)param_1 & 3;
        *(uint *)((int)param_1 - uVar3) =
             *(uint *)((int)param_1 - uVar3) & 0xffffffffU >> (4 - uVar3) * 8 | uVar4 << uVar3 * 8;
        uVar3 = (int)param_1 + 3U & 3;
        puVar1 = (uint *)((undefined1 *)((int)param_1 + 3U) + -uVar3);
        *puVar1 = *puVar1 & -1 << (uVar3 + 1) * 8 | uVar4 >> (3 - uVar3) * 8;
        param_1 = param_1 + 1;
      }
      for (param_3 = param_3 + 4; param_3 != 0; param_3 = param_3 + -1) {
        uVar3 = *param_2;
        param_2 = (uint *)((int)param_2 + 1);
        *(char *)param_1 = (char)uVar3;
        param_1 = (uint *)((int)param_1 + 1);
      }
    }
  }
  return puVar2;
}

