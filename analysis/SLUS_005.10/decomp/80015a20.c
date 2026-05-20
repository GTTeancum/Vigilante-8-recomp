// addr: 0x80015a20  name: FUN_80015a20

undefined4 FUN_80015a20(uint *param_1,uint param_2,undefined4 param_3,uint param_4)

{
  uint uVar1;
  uint *puVar2;
  uint *puVar3;
  uint *puVar4;
  uint *puVar5;
  uint in_t0;
  uint in_t1;
  uint in_t2;
  uint uVar6;
  
  if ((uRam000006ac & 0x7ff) != 0) {
    uVar6 = -uRam000006ac & 0x7ff;
    if ((int)param_2 < (int)(-uRam000006ac & 0x7ff)) {
      uVar6 = param_2;
    }
    FUN_80044c44(param_1,iRam0000069c + (uRam000006ac & 0x7ff),uVar6);
    param_1 = (uint *)((int)param_1 + uVar6);
    param_2 = param_2 - uVar6;
    uRam000006ac = uRam000006ac + uVar6;
  }
  for (; 0x7ff < (int)param_2; param_2 = param_2 - 0x800) {
    puVar3 = (uint *)FUN_800156d4();
    if ((((uint)puVar3 | (uint)param_1) & 3) == 0) {
      puVar4 = puVar3 + 0x200;
      puVar5 = param_1;
      do {
        param_4 = *puVar3;
        in_t0 = puVar3[1];
        in_t1 = puVar3[2];
        in_t2 = puVar3[3];
        *puVar5 = param_4;
        puVar5[1] = in_t0;
        puVar5[2] = in_t1;
        puVar5[3] = in_t2;
        puVar3 = puVar3 + 4;
        puVar5 = puVar5 + 4;
      } while (puVar3 != puVar4);
    }
    else {
      puVar4 = puVar3 + 0x200;
      puVar5 = param_1;
      do {
        uVar6 = (int)puVar3 + 3U & 3;
        uVar1 = (uint)puVar3 & 3;
        param_4 = (*(int *)(((int)puVar3 + 3U) - uVar6) << (3 - uVar6) * 8 |
                  param_4 & 0xffffffffU >> (uVar6 + 1) * 8) & -1 << (4 - uVar1) * 8 |
                  *(uint *)((int)puVar3 - uVar1) >> uVar1 * 8;
        uVar6 = (int)puVar3 + 7U & 3;
        uVar1 = (uint)(puVar3 + 1) & 3;
        in_t0 = (*(int *)(((int)puVar3 + 7U) - uVar6) << (3 - uVar6) * 8 |
                in_t0 & 0xffffffffU >> (uVar6 + 1) * 8) & -1 << (4 - uVar1) * 8 |
                *(uint *)((int)(puVar3 + 1) - uVar1) >> uVar1 * 8;
        uVar6 = (int)puVar3 + 0xbU & 3;
        uVar1 = (uint)(puVar3 + 2) & 3;
        in_t1 = (*(int *)(((int)puVar3 + 0xbU) - uVar6) << (3 - uVar6) * 8 |
                in_t1 & 0xffffffffU >> (uVar6 + 1) * 8) & -1 << (4 - uVar1) * 8 |
                *(uint *)((int)(puVar3 + 2) - uVar1) >> uVar1 * 8;
        uVar6 = (int)puVar3 + 0xfU & 3;
        uVar1 = (uint)(puVar3 + 3) & 3;
        in_t2 = (*(int *)(((int)puVar3 + 0xfU) - uVar6) << (3 - uVar6) * 8 |
                in_t2 & 0xffffffffU >> (uVar6 + 1) * 8) & -1 << (4 - uVar1) * 8 |
                *(uint *)((int)(puVar3 + 3) - uVar1) >> uVar1 * 8;
        uVar6 = (int)puVar5 + 3U & 3;
        puVar2 = (uint *)(((int)puVar5 + 3U) - uVar6);
        *puVar2 = *puVar2 & -1 << (uVar6 + 1) * 8 | param_4 >> (3 - uVar6) * 8;
        uVar6 = (uint)puVar5 & 3;
        *(uint *)((int)puVar5 - uVar6) =
             *(uint *)((int)puVar5 - uVar6) & 0xffffffffU >> (4 - uVar6) * 8 | param_4 << uVar6 * 8;
        uVar6 = (int)puVar5 + 7U & 3;
        puVar2 = (uint *)(((int)puVar5 + 7U) - uVar6);
        *puVar2 = *puVar2 & -1 << (uVar6 + 1) * 8 | in_t0 >> (3 - uVar6) * 8;
        uVar6 = (uint)(puVar5 + 1) & 3;
        puVar2 = (uint *)((int)(puVar5 + 1) - uVar6);
        *puVar2 = *puVar2 & 0xffffffffU >> (4 - uVar6) * 8 | in_t0 << uVar6 * 8;
        uVar6 = (int)puVar5 + 0xbU & 3;
        puVar2 = (uint *)(((int)puVar5 + 0xbU) - uVar6);
        *puVar2 = *puVar2 & -1 << (uVar6 + 1) * 8 | in_t1 >> (3 - uVar6) * 8;
        uVar6 = (uint)(puVar5 + 2) & 3;
        puVar2 = (uint *)((int)(puVar5 + 2) - uVar6);
        *puVar2 = *puVar2 & 0xffffffffU >> (4 - uVar6) * 8 | in_t1 << uVar6 * 8;
        uVar6 = (int)puVar5 + 0xfU & 3;
        puVar2 = (uint *)(((int)puVar5 + 0xfU) - uVar6);
        *puVar2 = *puVar2 & -1 << (uVar6 + 1) * 8 | in_t2 >> (3 - uVar6) * 8;
        uVar6 = (uint)(puVar5 + 3) & 3;
        puVar2 = (uint *)((int)(puVar5 + 3) - uVar6);
        *puVar2 = *puVar2 & 0xffffffffU >> (4 - uVar6) * 8 | in_t2 << uVar6 * 8;
        puVar3 = puVar3 + 4;
        puVar5 = puVar5 + 4;
      } while (puVar3 != puVar4);
    }
    param_1 = param_1 + 0x200;
    uRam000006ac = uRam000006ac + 0x800;
  }
  if (param_2 != 0) {
    iRam0000069c = FUN_800156d4();
    FUN_80044c44(param_1,iRam0000069c,param_2);
    uRam000006ac = uRam000006ac + param_2;
  }
  return 1;
}

