// addr: 0x8002d82c  name: FUN_8002d82c

undefined4 FUN_8002d82c(uint *param_1,undefined4 *param_2)

{
  char cVar1;
  byte bVar2;
  undefined1 uVar3;
  undefined4 uVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  uint uVar9;
  uint *puVar10;
  undefined4 *puVar11;
  undefined8 uVar12;
  uint local_78;
  uint local_74;
  uint local_70;
  int local_68;
  int local_64;
  int local_60;
  
  puVar10 = (uint *)*param_2;
  if (*(char *)(param_2[4] + 4) != '\x03') {
    FUN_8001f5a0(param_1,param_2);
    cVar1 = (char)puVar10[1];
    if (cVar1 == '\a') {
      if (*(short *)((int)param_1 + 0x11e) != 0) {
        FUN_8002c018(param_1);
        return 0;
      }
      if ((*puVar10 & 0x800000) != 0) {
        uVar9 = puVar10[0x20];
        if (((*(char *)(uVar9 + 0xb6) == '\0') ||
            ((int)*(short *)((int)param_1 + 6) != (int)*(char *)(uVar9 + 0xb7))) ||
           (*(ushort *)((int)puVar10 + 10) == (ushort)*(byte *)(uVar9 + 0xb8))) {
          uVar3 = 0x1e;
          if (*(char *)(uVar9 + 0xb9) != '\0') {
            uVar3 = 1;
          }
          *(undefined1 *)(uVar9 + 0xb6) = uVar3;
          *(undefined1 *)(uVar9 + 0xb7) = *(undefined1 *)((int)param_1 + 6);
          uVar3 = *(undefined1 *)((int)puVar10 + 10);
        }
        else {
          bVar2 = *(char *)(uVar9 + 0xb9) + 1;
          *(byte *)(uVar9 + 0xb9) = bVar2;
          *(undefined1 *)(uVar9 + 0xb6) = 0x1e;
          uVar6 = (uint)(ushort)puVar10[3] * (bVar2 + 1);
          uVar5 = 0xffff;
          if (uVar6 < 0xffff) {
            uVar5 = uVar6;
          }
          uVar3 = *(undefined1 *)((int)puVar10 + 10);
          *(short *)(puVar10 + 3) = (short)uVar5;
        }
        *(undefined1 *)(uVar9 + 0xb8) = uVar3;
        uVar6 = *puVar10;
        *puVar10 = uVar6 & 0xff7fffff;
        if (((short)param_1[3] == 0) && ((uVar6 & 0x1000000) != 0)) {
          if (*(short *)(uVar9 + 6) < 0) {
            uVar4 = FUN_8002b940(param_1);
            sprintf((char *)&local_68,"%s TOTALED!",uVar4);
            if (iRam00000010 == 0) {
              iVar7 = 0;
            }
            else {
              iVar7 = -(int)*(short *)(uVar9 + 6);
            }
            FUN_800129e8(iVar7,&local_68);
          }
          FUN_8002bc18(param_1);
          FUN_8003fea8(param_1 + 9,0x80000ff);
          param_1[0x25] = 50000;
          param_1[0x21] = param_1[0x21] - 0xee680;
          FUN_8002d44c(uVar9);
          *(char *)(uVar9 + 0xbb) = *(char *)(uVar9 + 0xbb) + '\x01';
        }
      }
      FUN_8002c6fc(param_1,-(uint)(ushort)puVar10[3],param_2 + 5,1);
      return 0;
    }
    if (cVar1 == '\x01') {
      if ((*puVar10 & 0x10000) == 0) {
        iVar7 = FUN_80016a20(puVar10 + 0x22);
        uVar9 = puVar10[0x15];
        if ((int)uVar9 < 0) {
          uVar9 = uVar9 + 0xff;
        }
        iVar7 = iVar7 * ((int)uVar9 >> 8);
      }
      else {
        uVar9 = puVar10[0x15];
        if ((int)uVar9 < 0) {
          uVar9 = uVar9 + 0xff;
        }
        iVar7 = puVar10[0x21] * ((int)uVar9 >> 8);
      }
      if (iVar7 < 0) {
        iVar7 = iVar7 + 0xfff;
      }
      uVar9 = (uint)(ushort)((ushort)param_1[3] >> 2);
      if (iVar7 >> 0xc < (int)uVar9) {
        uVar9 = iVar7 >> 0xc;
      }
      *puVar10 = *puVar10 | 0x20;
      FUN_8002c958(param_1,-uVar9,param_2 + 5,0);
      return 0;
    }
    if (cVar1 == '\x02') {
      local_68 = param_1[0x20] * (uint)(*(ushort *)((int)param_1 + 0xa2) >> 6) -
                 puVar10[0x20] * (uint)(*(ushort *)((int)puVar10 + 0xa2) >> 6);
      local_64 = param_1[0x21] * (uint)(*(ushort *)((int)param_1 + 0xa2) >> 6) -
                 puVar10[0x21] * (uint)(*(ushort *)((int)puVar10 + 0xa2) >> 6);
      local_60 = param_1[0x22] * (uint)(*(ushort *)((int)param_1 + 0xa2) >> 6) -
                 puVar10[0x22] * (uint)(*(ushort *)((int)puVar10 + 0xa2) >> 6);
      uVar12 = FUN_80017240(&local_68,param_2 + 8);
      uVar9 = (uint)uVar12 >> 0xd | (int)((ulonglong)uVar12 >> 0x20) << 0x13;
      if ((int)uVar9 < 0) {
        iVar7 = (int)uVar9 / (int)(uint)(*(ushort *)((int)param_1 + 0xa2) >> 6);
        FUN_800434d0(param_1 + 4,param_2 + 8,&local_78);
        uVar6 = -(param_2[0xc] * 2 + iVar7);
        iVar8 = (int)uVar6 >> 0x1f;
        local_78 = (uint)((ulonglong)local_78 * (ulonglong)uVar6) >> 0xc |
                   ((int)((ulonglong)local_78 * (ulonglong)uVar6 >> 0x20) + local_78 * iVar8 +
                   uVar6 * ((int)local_78 >> 0x1f)) * 0x100000;
        local_74 = (uint)((ulonglong)local_74 * (ulonglong)uVar6) >> 0xc |
                   ((int)((ulonglong)local_74 * (ulonglong)uVar6 >> 0x20) + local_74 * iVar8 +
                   uVar6 * ((int)local_74 >> 0x1f)) * 0x100000;
        local_70 = (uint)((ulonglong)local_70 * (ulonglong)uVar6) >> 0xc |
                   ((int)((ulonglong)local_70 * (ulonglong)uVar6 >> 0x20) + local_70 * iVar8 +
                   uVar6 * ((int)local_70 >> 0x1f)) * 0x100000;
        FUN_80017594(param_1,&local_78,param_2 + 5);
        if (iVar7 < 0) {
          iVar7 = iVar7 + 0x1fff;
        }
        if (iVar7 >> 0xd < -8) {
          FUN_8002c958(param_1,iVar7 >> 0xd,param_2 + 5,1);
          if (*(short *)((int)param_1 + 6) < 0) {
            FUN_80012028(~(int)*(short *)((int)param_1 + 6),10,0xc0,0,0x40);
          }
        }
        puVar11 = param_2 + 5;
        FUN_80043408(param_1 + 4,puVar11,puVar11);
        FUN_800435c0(puVar10 + 4,puVar11,puVar11);
        iVar7 = (int)uVar9 / (int)(uint)(*(ushort *)((int)puVar10 + 0xa2) >> 6);
        FUN_800434d0(puVar10 + 4,param_2 + 8,&local_78);
        uVar9 = param_2[0xc] * 2 + iVar7;
        iVar8 = (int)uVar9 >> 0x1f;
        local_78 = (uint)((ulonglong)local_78 * (ulonglong)uVar9) >> 0xc |
                   ((int)((ulonglong)local_78 * (ulonglong)uVar9 >> 0x20) + local_78 * iVar8 +
                   uVar9 * ((int)local_78 >> 0x1f)) * 0x100000;
        local_74 = (uint)((ulonglong)local_74 * (ulonglong)uVar9) >> 0xc |
                   ((int)((ulonglong)local_74 * (ulonglong)uVar9 >> 0x20) + local_74 * iVar8 +
                   uVar9 * ((int)local_74 >> 0x1f)) * 0x100000;
        local_70 = (uint)((ulonglong)local_70 * (ulonglong)uVar9) >> 0xc |
                   ((int)((ulonglong)local_70 * (ulonglong)uVar9 >> 0x20) + local_70 * iVar8 +
                   uVar9 * ((int)local_70 >> 0x1f)) * 0x100000;
        FUN_80017594(puVar10,&local_78,puVar11);
        if (iVar7 < 0) {
          iVar7 = iVar7 + 0x1fff;
        }
        if (iVar7 >> 0xd < -8) {
          FUN_8002c958(puVar10,iVar7 >> 0xd,puVar11,1);
          if (*(short *)((int)puVar10 + 6) < 0) {
            FUN_80012028(~(int)*(short *)((int)puVar10 + 6),10,0xc0,0,0x40);
          }
        }
        if (((*param_1 & 0x8000) == 0) && (0x1c9 < (int)param_1[0x23])) {
          uVar4 = FUN_8004410c();
          FUN_8004483c(uVar4,uRam000005f8,0x1e,puVar10 + 0x12);
        }
      }
      *param_1 = *param_1 | 0x18000;
      return 1;
    }
    if ((((uint *)param_1[0x1d] == puVar10) || ((uint *)param_1[0x1e] == puVar10)) &&
       (*(short *)((int)param_2 + 0x22) < -0x800)) {
      return 0;
    }
    uVar12 = FUN_80017240(param_1 + 0x20,param_2 + 8);
    uVar9 = (uint)uVar12 >> 0xd | (int)((ulonglong)uVar12 >> 0x20) << 0x13;
    if (-1 < (int)uVar9) {
      return 0;
    }
    iVar7 = -uVar9;
    if (iVar7 < 0) {
      iVar7 = iVar7 + 0x3fff;
    }
    iVar7 = (iVar7 >> 0xe) * (uint)*(ushort *)((int)param_1 + 0xa2);
    if (iVar7 < 0) {
      iVar7 = iVar7 + 0xfff;
    }
    if ((code *)puVar10[0x19] == (code *)0x0) {
      iVar7 = 0;
    }
    else {
      iVar7 = (*(code *)puVar10[0x19])(puVar10,8,iVar7 >> 0xc);
    }
    if (iVar7 != 0) {
      return 0;
    }
    FUN_800434d0(param_1 + 4,param_2 + 8,&local_78);
    uVar6 = -(param_2[0xc] + uVar9);
    iVar7 = (int)uVar6 >> 0x1f;
    local_78 = (uint)((ulonglong)local_78 * (ulonglong)uVar6) >> 0xc |
               ((int)((ulonglong)local_78 * (ulonglong)uVar6 >> 0x20) + local_78 * iVar7 +
               uVar6 * ((int)local_78 >> 0x1f)) * 0x100000;
    local_74 = (uint)((ulonglong)local_74 * (ulonglong)uVar6) >> 0xc |
               ((int)((ulonglong)local_74 * (ulonglong)uVar6 >> 0x20) + local_74 * iVar7 +
               uVar6 * ((int)local_74 >> 0x1f)) * 0x100000;
    local_70 = (uint)((ulonglong)local_70 * (ulonglong)uVar6) >> 0xc |
               ((int)((ulonglong)local_70 * (ulonglong)uVar6 >> 0x20) + local_70 * iVar7 +
               uVar6 * ((int)local_70 >> 0x1f)) * 0x100000;
    FUN_80017594(param_1,&local_78,param_2 + 5);
    if ((*param_1 & 0x8000) == 0) {
      if ((int)uVar9 < 0) {
        uVar9 = uVar9 + 0x3fff;
      }
      iVar7 = -(uint)(ushort)puVar10[3];
      if ((int)-(uint)(ushort)puVar10[3] < (int)uVar9 >> 0xe) {
        iVar7 = (int)uVar9 >> 0xe;
      }
      FUN_8002c958(param_1,iVar7,param_2 + 5,0);
      if (*(short *)((int)param_1 + 6) < 0) {
        FUN_80012068(~(int)*(short *)((int)param_1 + 6),0xff,0,0x40);
      }
      if (0x1c9 < (int)param_1[0x23]) {
        uVar6 = (uint)(*(ushort *)((int)puVar10 + 6) >> 4);
        uVar9 = 0xb;
        if (uVar6 < 0xb) {
          uVar9 = uVar6;
        }
        cVar1 = (&DAT_8001057c)[uVar9];
        if (cVar1 != -1) {
          uVar4 = FUN_8004410c();
          FUN_8004483c(uVar4,uRam000005f8,cVar1,puVar10 + 9);
        }
      }
    }
    *param_1 = *param_1 | 0x18000;
  }
  return 0;
}

