// addr: 0x800506a0  name: SYS_OBJ_149C

void SYS_OBJ_149C(int param_1,ushort *param_2)

{
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  short sVar4;
  undefined4 uVar5;
  char cVar6;
  undefined4 local_18;
  undefined4 local_14;
  
  uVar5 = SYS_OBJ_172C((int)(short)*param_2,(int)(short)param_2[1]);
  *(undefined4 *)(param_1 + 4) = uVar5;
  uVar5 = SYS_OBJ_17C4((int)(((uint)param_2[2] + (uint)*param_2 + -1) * 0x10000) >> 0x10,
                       (int)(((uint)param_2[1] + (uint)param_2[3] + -1) * 0x10000) >> 0x10);
  *(undefined4 *)(param_1 + 8) = uVar5;
  uVar5 = SYS_OBJ_185C((int)(short)param_2[4],(int)(short)param_2[5]);
  *(undefined4 *)(param_1 + 0xc) = uVar5;
  uVar5 = SYS_OBJ_170C(*(undefined1 *)((int)param_2 + 0x17),(char)param_2[0xb],param_2[10]);
  *(undefined4 *)(param_1 + 0x10) = uVar5;
  uVar5 = SYS_OBJ_1878(param_2 + 6);
  *(undefined4 *)(param_1 + 0x14) = uVar5;
  *(undefined4 *)(param_1 + 0x18) = 0xe6000000;
  cVar6 = '\a';
  if ((char)param_2[0xc] != '\0') {
    uVar5 = *(undefined4 *)param_2;
    uVar1 = param_2[2];
    uVar2 = param_2[3];
    uVar3 = 0;
    if ((-1 < (short)uVar1) && (uVar3 = DAT_80065028 - 1, (int)(short)uVar1 <= DAT_80065028 + -1)) {
      uVar3 = uVar1;
    }
    if ((short)uVar2 < 0) {
      sVar4 = 0;
    }
    else {
      sVar4 = DAT_8006502a + -1;
      if ((int)(short)uVar2 <= DAT_8006502a + -1) {
        SYS_OBJ_15F8(uVar2);
        return;
      }
    }
    local_14 = CONCAT22(sVar4,uVar3);
    if (((*param_2 & 0x3f) != 0) || ((uVar3 & 0x3f) != 0)) {
      local_18 = CONCAT22(param_2[1] - param_2[5],*param_2 - param_2[4]);
      *(uint *)(param_1 + 0x1c) =
           (uint)*(byte *)((int)param_2 + 0x1b) << 0x10 | (uint)(byte)param_2[0xd] << 8 | 0x60000000
           | (uint)*(byte *)((int)param_2 + 0x19);
      *(undefined4 *)(param_1 + 0x20) = local_18;
      *(undefined4 *)(param_1 + 0x24) = local_14;
      SYS_OBJ_16F0();
      return;
    }
    cVar6 = '\n';
    *(uint *)(param_1 + 0x1c) =
         (uint)*(byte *)((int)param_2 + 0x1b) << 0x10 | (uint)(byte)param_2[0xd] << 8 | 0x2000000 |
         (uint)*(byte *)((int)param_2 + 0x19);
    *(undefined4 *)(param_1 + 0x20) = uVar5;
    *(undefined4 *)(param_1 + 0x24) = local_14;
  }
  *(char *)(param_1 + 3) = cVar6 + -1;
  return;
}

