// addr: 0x80055034  name: PADCMD_OBJ_3A0

undefined4 PADCMD_OBJ_3A0(int *param_1)

{
  undefined1 uVar1;
  byte bVar2;
  undefined4 uVar3;
  int iVar4;
  undefined1 *puVar5;
  
  bVar2 = *(byte *)((int)param_1 + 0x46);
  if (bVar2 == 3) {
    puVar5 = (undefined1 *)(param_1[1] + (uint)*(byte *)((int)param_1 + 0x47) * 5);
    *puVar5 = *(undefined1 *)(param_1[0xf] + 4);
    puVar5[1] = *(byte *)(param_1[0xf] + 5) & 0x7f;
    puVar5[2] = *(undefined1 *)(param_1[0xf] + 6);
    puVar5[3] = *(undefined1 *)(param_1[0xf] + 7);
    puVar5[4] = (char)((int)(uint)*(byte *)(param_1[0xf] + 5) >> 7);
    bVar2 = *(char *)((int)param_1 + 0x47) + 1;
    *(byte *)((int)param_1 + 0x47) = bVar2;
    if (*(byte *)((int)param_1 + 0xe9) <= bVar2) {
      *(undefined1 *)((int)param_1 + 0x47) = 0;
      *(undefined1 *)(param_1 + 0x12) = 0;
      uVar3 = PADCMD_OBJ_608();
      return uVar3;
    }
  }
  else if (bVar2 < 4) {
    if (bVar2 != 2) {
      uVar3 = PADCMD_OBJ_60C();
      return uVar3;
    }
    *(ushort *)((uint)*(byte *)((int)param_1 + 0x47) * 2 + *param_1) =
         (ushort)*(byte *)(param_1[0xf] + 5) + (ushort)*(byte *)(param_1[0xf] + 4) * 0x100;
    bVar2 = *(char *)((int)param_1 + 0x47) + 1;
    *(byte *)((int)param_1 + 0x47) = bVar2;
    if (*(byte *)((int)param_1 + 0xe3) <= bVar2) {
      *(undefined1 *)((int)param_1 + 0x47) = 0;
      uVar3 = PADCMD_OBJ_608();
      return uVar3;
    }
  }
  else {
    if (bVar2 != 4) {
      uVar3 = PADCMD_OBJ_60C();
      return uVar3;
    }
    puVar5 = (undefined1 *)(param_1[2] + (uint)*(byte *)((int)param_1 + 0x47) * 8);
    if ((char)param_1[0x12] == '\0') {
      uVar1 = *(undefined1 *)(param_1[0xf] + 4);
      *(undefined1 *)(param_1 + 0x12) = uVar1;
      *puVar5 = uVar1;
      iVar4 = param_1[0xf];
      if (*(char *)((int)param_1 + 0x47) == '\0') {
        uVar3 = PADCMD_OBJ_538();
        return uVar3;
      }
      DAT_800a4c88 = (undefined1 *)(*(int *)(puVar5 + -4) + ((byte)puVar5[-8] + 3 & 0x1fc));
      *(undefined1 **)(puVar5 + 4) = DAT_800a4c88;
      uVar3 = PADCMD_OBJ_560(2,param_1,iVar4 + 5);
      return uVar3;
    }
    puVar5 = (undefined1 *)(param_1[0xf] + 2);
    iVar4 = 5;
    do {
      iVar4 = iVar4 + -1;
      if ((char)param_1[0x12] == '\0') goto PADCMD_OBJ_5CC;
      uVar1 = *puVar5;
      puVar5 = puVar5 + 1;
      *DAT_800a4c88 = uVar1;
      DAT_800a4c88 = DAT_800a4c88 + 1;
      *(char *)(param_1 + 0x12) = (char)param_1[0x12] + -1;
    } while (iVar4 != -1);
    if ((char)param_1[0x12] == '\0') {
PADCMD_OBJ_5CC:
      bVar2 = *(char *)((int)param_1 + 0x47) + 1;
      *(byte *)((int)param_1 + 0x47) = bVar2;
      if (*(byte *)((int)param_1 + 0xea) <= bVar2) {
        *(undefined1 *)((int)param_1 + 0x49) = 6;
        *(undefined1 *)((int)param_1 + 0x46) = 0xfe;
        uVar3 = PADCMD_OBJ_60C();
        return uVar3;
      }
      *(undefined1 *)(param_1 + 0x12) = 0;
      uVar3 = PADCMD_OBJ_5C4();
      return uVar3;
    }
  }
  return 0;
}

